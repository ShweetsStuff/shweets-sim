cmake_minimum_required(VERSION 3.27)
project(shweets-sim LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

if(MSVC)
    set(CMAKE_SUPPRESS_REGENERATION ON)
    set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD OFF)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        if(NOT CMAKE_BUILD_TYPE)
            set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
        endif()
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
    endif()

    if(MSVC)
        set(CMAKE_C_FLAGS_DEBUG "/Od /Zi" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "/W4 /permissive- /Od /Zi" CACHE STRING "" FORCE)
        set(CMAKE_C_FLAGS_RELEASE "/O2 /DNDEBUG" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "/W4 /permissive- /O2 /DNDEBUG" CACHE STRING "" FORCE)
    else()
        set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wpedantic -O0 -g" CACHE STRING "" FORCE)
        set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wpedantic -O3 -DNDEBUG" CACHE STRING "" FORCE)
    endif()
endif()

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
if (NOT WIN32)
    set(SDL_ALSA ON CACHE BOOL "" FORCE)
    set(SDL_ALSA_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_JACK ON CACHE BOOL "" FORCE)
    set(SDL_JACK_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_PIPEWIRE ON CACHE BOOL "" FORCE)
    set(SDL_PIPEWIRE_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_SNDIO ON CACHE BOOL "" FORCE)
    set(SDL_SNDIO_SHARED ON CACHE BOOL "" FORCE)
endif ()
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_TRAY OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

set(SDLMIXER_DEPS_SHARED OFF CACHE BOOL "" FORCE)
set(SDLMIXER_AIFF OFF CACHE BOOL "" FORCE)
set(SDLMIXER_AU OFF CACHE BOOL "" FORCE)
set(SDLMIXER_FLAC_LIBFLAC OFF CACHE BOOL "" FORCE)
set(SDLMIXER_FLAC_DRFLAC OFF CACHE BOOL "" FORCE)
set(SDLMIXER_GME OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MOD_XMP OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MP3_DRMP3 OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MIDI_FLUIDSYNTH OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MIDI_TIMIDITY OFF CACHE BOOL "" FORCE)
set(SDLMIXER_OPUS ON CACHE BOOL "" FORCE)
set(SDLMIXER_VOC OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_STB ON CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_TREMOR OFF CACHE BOOL "" FORCE)
set(SDLMIXER_WAVE ON CACHE BOOL "" FORCE)
set(SDLMIXER_WAVPACK OFF CACHE BOOL "" FORCE)
set(SDLMIXER_TEST OFF CACHE BOOL "" FORCE)
set(SDLMIXER_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL_mixer EXCLUDE_FROM_ALL)

set(PHYSFS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_STATIC ON  CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_WX_TEST OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_ZIP ON  CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_7Z  OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_GRP OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_HOG OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_PAK OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_WAD OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_DIR ON CACHE BOOL "" FORCE)
set(PHYSFS_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(external/physfs)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(TINYXML2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyxml2)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)
set(PHYSFS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/physfs)

set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
)

set (TINYXML2_SOURCES ${TINYXML2_DIR}/tinyxml2.cpp)

file(GLOB PROJECT_SRC CONFIGURE_DEPENDS
	        include/*.cpp
	        src/*.cpp
	        src/render/*.cpp
	        src/resource/*.cpp
	        src/resource/xml/*.cpp
	        src/state/*.cpp
	        src/state/play/*.cpp
	        src/state/play/arcade/*.cpp
	        src/state/select/*.cpp
	        src/entity/*.cpp
	        src/window/*.cpp
	        src/util/*.cpp
	        src/util/imgui/*.cpp
)

add_executable(${PROJECT_NAME}
        ${PROJECT_SRC}
        ${IMGUI_SOURCES}
        ${TINYXML2_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.cpp
)

if(WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    enable_language(RC)
    target_sources(${PROJECT_NAME} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/shweets-sim.rc"
            "${CMAKE_CURRENT_SOURCE_DIR}/Icon.ico"
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:DEBUG=1>"
        "$<$<NOT:$<CONFIG:Debug>>:DEBUG=0>"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL_mixer/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${GLM_DIR}
        ${TINYXML2_DIR}
        include/glad
        include/KHR
        include
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static SDL3_mixer::SDL3_mixer-static physfs-static)
if (WIN32 AND TARGET SDL3::SDL3main)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3main)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
                "$<$<CONFIG:Release>:/O2>"
                "$<$<CONFIG:Release>:/DNDEBUG>"
        )
        if(WIN32)
            target_link_options(${PROJECT_NAME} PRIVATE
                    "/SUBSYSTEM:WINDOWS"
                    "/ENTRY:mainCRTStartup"
            )
        endif()
        target_link_options(${PROJECT_NAME} PRIVATE
                "$<$<CONFIG:Release>:/DEBUG:NONE>"
                "$<$<CONFIG:Release>:/INCREMENTAL:NO>"
                "$<$<CONFIG:Release>:/OPT:REF>"
                "$<$<CONFIG:Release>:/OPT:ICF>"
        )
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE
                "$<$<CONFIG:Release>:-O3>"
                "$<$<CONFIG:Release>:-DNDEBUG>"
        )
        if(WIN32)
            target_link_options(${PROJECT_NAME} PRIVATE
                    "-mwindows"
            )
        endif()
        target_link_options(${PROJECT_NAME} PRIVATE
                "$<$<CONFIG:Release>:-s>"
        )
    endif()
endif()

set(PROJECT_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(PROJECT_RESOURCES_BINARY_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/resources")
if(EXISTS "${PROJECT_RESOURCES_DIR}")
    file(GLOB_RECURSE PROJECT_RESOURCE_FILES CONFIGURE_DEPENDS
            "${PROJECT_RESOURCES_DIR}/*")
    list(FILTER PROJECT_RESOURCE_FILES EXCLUDE REGEX ".*/characters/[^/]+\\.zip$")
    set(COPY_RESOURCES_STAMP "${CMAKE_BINARY_DIR}/CMakeFiles/copy_resources.stamp")
    add_custom_command(
            OUTPUT "${COPY_RESOURCES_STAMP}"
            COMMAND ${CMAKE_COMMAND}
                    -DSRC_DIR=${PROJECT_RESOURCES_DIR}
                    -DDST_DIR=${PROJECT_RESOURCES_BINARY_DIR}
                    -DSTAMP_FILE=${COPY_RESOURCES_STAMP}
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_resources.cmake"
            DEPENDS ${PROJECT_RESOURCE_FILES}
                    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_resources.cmake"
            COMMENT "Copying resources directory"
            VERBATIM
    )
    add_custom_target(copy_resources ALL
            DEPENDS "${COPY_RESOURCES_STAMP}")
    add_dependencies(${PROJECT_NAME} copy_resources)
    set(HAS_PROJECT_RESOURCES TRUE)
else()
    set(HAS_PROJECT_RESOURCES FALSE)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
if(HAS_PROJECT_RESOURCES)
    install(DIRECTORY "${PROJECT_RESOURCES_DIR}/" DESTINATION resources)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EMSCRIPTEN_SHELL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/web/index.html")
    target_link_options(${PROJECT_NAME} PRIVATE
            "-sMIN_WEBGL_VERSION=2"
            "-sMAX_WEBGL_VERSION=2"
            "-sFULL_ES2=1"
            "-sUSE_OGG=1"
            "-sUSE_VORBIS=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sFORCE_FILESYSTEM=1"
            "-sASYNCIFY"
            "-lidbfs.js"
            "--shell-file"
            "${EMSCRIPTEN_SHELL_FILE}"
    )
    if(HAS_PROJECT_RESOURCES)
        target_link_options(${PROJECT_NAME} PRIVATE
                "--preload-file"
                "${PROJECT_RESOURCES_BINARY_DIR}@resources"
        )
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME "index"
            SUFFIX ".html")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                    -DBIN_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/create_index_zip.cmake"
            COMMENT "Creating shweets-sim-web.zip from Emscripten output")
else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
endif()

if(WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                    -DBIN_ROOT="${CMAKE_BINARY_DIR}/bin"
                    -DTARGET_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                    -DEXE_FILE="$<TARGET_FILE_NAME:${PROJECT_NAME}>"
                    -DPACKAGE_NAME="shweets-sim-win32"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/create_windows_zip.cmake"
            COMMENT "Creating shweets-sim-win32 package")
endif()

message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
