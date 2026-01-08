# 3D Fantasy Forest Scene

A real-time 3D graphics application built with OpenGL that renders an interactive fantasy forest environment. The scene features a whimsical mushroom house, trees, fairy character, and other nature elements in a stylized outdoor setting.

## Features

- **Free-look Camera Control**: Full 6-DOF camera movement with mouse look (pitch and yaw)
- **WASD Movement**: Smooth, frame-rate independent keyboard navigation
- **Phong Lighting Model**: Realistic lighting and shading on all scene objects
- **Skybox Rendering**: Immersive sky environment that follows the camera (WIP)
- **Multiple 3D Models**: Loaded via Assimp, including GLTF format support
- **Textured Surfaces**: Ground plane with texture mapping (WIP)

## Controls

- **W/A/S/D**: Move forward/left/backward/right
- **Mouse**: Look around (first-person view)
- **ESC**: Exit application

## Technical Details

- Built with C++ and OpenGL 3.3
- Uses SFML for windowing and input handling
- Assimp for 3D model loading
- Custom shader programs for Phong lighting and skybox rendering
- Hierarchical scene graph with Object3D transformations

## Requirements

- OpenGL 3.3 or higher
- SFML
- Assimp
- GLAD
- GLM (OpenGL Mathematics)
