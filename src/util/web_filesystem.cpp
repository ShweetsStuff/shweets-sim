#include "web_filesystem.hpp"

#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
  #include <emscripten/emscripten.h>
#endif

namespace game::util::web_filesystem
{
#ifdef __EMSCRIPTEN__
  EM_JS(void, idbfs_init_async, (), {
    Module.filesystemReady = 0;
    try
    {
      FS.mkdir('/shweets-sim');
    }
    catch (e)
    {
    }
    FS.mount(IDBFS, {}, '/shweets-sim');
    FS.syncfs(
        true, function(err) {
          if (err) console.error('IDBFS init sync failed', err);
          Module.filesystemReady = 1;
        });
  });

  EM_JS(int, idbfs_ready, (), { return Module.filesystemReady ? 1 : 0; });

  EM_JS(void, idbfs_flush_async, (), {
    FS.syncfs(
        false, function(err) {
          if (err) console.error('IDBFS flush failed', err);
        });
  });
#endif

  void init_and_wait()
  {
#ifdef __EMSCRIPTEN__
    idbfs_init_async();
    while (!idbfs_ready())
      emscripten_sleep(16);
#endif
  }

  void flush_async()
  {
#ifdef __EMSCRIPTEN__
    idbfs_flush_async();
#endif
  }
}
