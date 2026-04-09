#include "loader.hpp"
#include "util/imgui/widget.hpp"

#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
#else
  #include <glad/glad.h>
#endif

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <format>
#include <imgui.h>

#include "log.hpp"
#include "util/math.hpp"

#include <SDL3_mixer/SDL_mixer.h>

#include <physfs.h>

#include "resource/audio.hpp"
#include "resource/xml/settings.hpp"
#include "util/imgui/style.hpp"
#include "util/preferences.hpp"

#ifdef __EMSCRIPTEN__

  #include "util/web_filesystem.hpp"

constexpr auto GL_VERSION_MAJOR = 3;
constexpr auto GL_VERSION_MINOR = 0;
constexpr auto GLSL_VERSION = "#version 300 es";

#else
constexpr auto GL_VERSION_MAJOR = 3;
constexpr auto GL_VERSION_MINOR = 3;
constexpr auto GLSL_VERSION = "#version 330";
#endif

using namespace game::util;

namespace game
{

  Loader::Loader(int argc, const char** argv)
  {

#ifdef __EMSCRIPTEN__
    util::web_filesystem::init_and_wait();
#endif

    settings = resource::xml::Settings(preferences::path() / "settings.xml");

    logger.info("Initializing...");

    if (!PHYSFS_init((argc > 0 && argv && argv[0]) ? argv[0] : "shweets-sim"))
    {
      logger.fatal(std::format("Failed to initialize PhysicsFS: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
      isError = true;
      return;
    }

    PHYSFS_setWriteDir(nullptr);

    logger.info("Initialized PhysFS");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
      logger.fatal(std::format("Failed to initialize SDL: {}", SDL_GetError()));
      isError = true;
      return;
    }

    logger.info("Initialized SDL");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR);
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifdef __EMSCRIPTEN__
    static constexpr glm::vec2 SIZE = {1600, 900};
    window = SDL_CreateWindow("Shweet's Sim", SIZE.x, SIZE.y, SDL_WINDOW_OPENGL);
#else

    SDL_PropertiesID windowProperties = SDL_CreateProperties();

    SDL_SetStringProperty(windowProperties, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Shweet's Sim");
    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, (long)settings.windowSize.x);
    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, (long)settings.windowSize.y);

    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_X_NUMBER,
                          settings.windowPosition.x == 0 ? SDL_WINDOWPOS_CENTERED : (long)settings.windowPosition.x);
    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_Y_NUMBER,
                          settings.windowPosition.y == 0 ? SDL_WINDOWPOS_CENTERED : (long)settings.windowPosition.y);

    SDL_SetBooleanProperty(windowProperties, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
    SDL_SetBooleanProperty(windowProperties, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetBooleanProperty(windowProperties, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);

    window = SDL_CreateWindowWithProperties(windowProperties);
#endif

    if (!window)
    {
      logger.fatal(std::format("Failed to initialize window: {}", SDL_GetError()));
      isError = true;
      return;
    }

    logger.info("Initialized window");

    context = SDL_GL_CreateContext(window);

    if (!context)
    {
      logger.fatal(std::format("Failed to initialize GL context: {}", SDL_GetError()));
      isError = true;
      return;
    }

    if (!SDL_GL_MakeCurrent(window, context))
    {
      logger.fatal(std::format("Failed to make GL context current: {}", SDL_GetError()));
      isError = true;
      return;
    }

#ifndef __EMSCRIPTEN__
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
      logger.fatal("Failed to initialize GLAD");
      isError = true;
      return;
    }

    logger.info("Initialized GLAD");
#endif

    glEnable(GL_BLEND);
    glLineWidth(2.0f);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    logger.info(std::format("Initialized GL context: {}", (const char*)glGetString(GL_VERSION)));

    SDL_GL_SetSwapInterval(1);
    SDL_GL_MakeCurrent(window, context);

    if (!MIX_Init())
    {
      logger.fatal(std::format("Failed to initialize SDL mixer: {}", SDL_GetError()));
      isError = true;
      return;
    }

    logger.info("Initialized SDL mixer");

    IMGUI_CHECKVERSION();
    ImGuiContext* imguiContext = ImGui::CreateContext();

    if (!imguiContext)
    {
      logger.fatal("Failed to initialize Dear ImGui");
      isError = true;
      return;
    }

    logger.info("Initialized Dear ImGui");

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForOpenGL(window, context))
    {
      logger.fatal("Failed to initialize Dear ImGui SDL3 backend");
      ImGui::DestroyContext(imguiContext);
      isError = true;
      return;
    }

    logger.info("Initialized Dear ImGui SDL3 backend");

    if (!ImGui_ImplOpenGL3_Init(GLSL_VERSION))
    {
      logger.fatal("Failed to initialize Dear ImGui OpenGL backend");
      ImGui_ImplSDL3_Shutdown();
      ImGui::DestroyContext(imguiContext);
      isError = true;
      return;
    }

    logger.info("Initialized Dear ImGui OpenGL backend");

    imgui::style::color_set(settings.color);
    imgui::style::widget_set();
    math::random_seed_set();
    resource::Audio::volume_set((float)settings.volume / 100);
  }

  Loader::~Loader()
  {
    PHYSFS_deinit();

    if (ImGui::GetCurrentContext())
    {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL3_Shutdown();
      ImGui::DestroyContext();
    }

    if (context) SDL_GL_DestroyContext(context);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();

    logger.info("Exiting...");
  }
};
