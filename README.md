# Lone Palm Island 
![beauty shot](/beauty-shot.png)
This demo is a small personal capstone - a snapshot of all I've learned from the [LearnOpenGL](https://learnopengl.com/) online textbook (thus far). Some quick notes on the source code, assets, and shaders: 

### Source 
The majority of rendering logic lives in `main.cpp` with a few steps abstracted to other files within `utils`. The following are borrowed directly from [LearnOpenGL](https://learnopengl.com/) with small modifications: 

- `camera.h` `camera.cpp` &nbsp; &nbsp; &rarr; &nbsp; &nbsp; simple fly-like camera system
- `shader.h` `shader.cpp` &nbsp; &nbsp; &rarr; &nbsp; &nbsp; shader init and use
- `mesh.h` `model.h` &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &rarr; &nbsp; &nbsp;  model processor 

Files beginning with [`stb`](https://github.com/nothings/stb) are header dependencies that process images/textures and write framebuffer data to image files.

### Assets
All models were designed in [Blender](https://www.blender.org/). Textures for the **_palm tree_** were sourced from items around my apartment that I photographed and modified digitally. The **_island_**  was painted using Blender's Texture Paint tool. 

### Shaders 
Most of the magic lives in `water.frag`. If you are interested in designing a similar water shader, [ThinMatrix's OpenGL Water Tutorial](https://www.youtube.com/watch?v=HusvGeEDU_U) is an incredible resource. 

## Setup
You will need `CMake`, `vcpkg`, and `ninja` to build this demo.

***NOTE:*** this CMake preset requires an environment variable `VCPKG_ROOT` which should point to the root directory where your version of `vcpkg` is installed. 

### Dependencies 
- `glad` &nbsp; &nbsp; &nbsp; &nbsp; &rarr; &nbsp; &nbsp; dynamically loads OpenGL functions 
- `glfw3` &nbsp; &nbsp; &nbsp; &rarr; &nbsp; &nbsp; assists in window creation, context managment, and I/O
- `glm` &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &rarr; &nbsp; &nbsp; powerful math library 
- `assimp` &nbsp; &nbsp; &rarr; &nbsp; &nbsp; loads assets - 3D models, etc. 

### 1. Clone 
```
git clone https://github.com/jebef/lone-palm-island.git
cd lone-palm-island
```
### 2. Build with CMake 
```
cmake --preset release
cmake --build build/release
```
### 3. Run
```
./build/release/LonePalmIsland
```

- `WASD` and `mouse` for movement 
- `esc` to exit 

## Author 
**Wyatt Jebef** - rendering pipeline, shaders, 3D models, textures, build system.  
[wyatt-jebef.com](https://wyatt-jebef.com/)  

Last Updated: 11/18/25
