# Shweet's Sim

![Preview](screenshots/preview.png)

Interactive character simulation game.

[Resources](https://shweetz.net/files/games/feed-snivy/resources.zip)

## Clarification
This application was developed with the assistance of a large language model.

## Build

After cloning and enter the repository's directory, make sure to initialize the submodules:
```git submodule update --init --recursive```

### Windows
Visual Studio is recommended for build.

### Linux

```
mkdir build
cd build
cmake ..
make 
```

If using VSCode, several tasks are available to quickly run and build.

### Emscripten
Run the VSCode "web" task. Make sure you have the Emscripten SDK accessible at the EMSDK path variable.