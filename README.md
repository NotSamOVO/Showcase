# Advanced Ray Tracing Showcase

## Personal Information
*   **Full Name:** [Your Full Name]
*   **UtorID:** [Your UtorID]
*   **Student Number:** [Your Student Number]
*   **Assignment Augmented:** A3 (Ray Tracing)

## Instructions
1.  **Build the project:**
    *   Create a build directory: `mkdir build`
    *   Navigate to it: `cd build`
    *   Run CMake: `cmake .. -DCMAKE_BUILD_TYPE=Release`
    *   Build: `cmake --build . --config Release` (or `make` on Linux/Mac)
2.  **Run the executable:**
    *   Run `./raytracing` (or `.\Release\raytracing.exe` on Windows)
    *   The program will generate a file named `piece.ppm`.
3.  **View the output:**
    *   Open `piece.ppm` with a compatible image viewer or use the provided `convert_ppm.py` script to convert it to PNG.

## Description
This project is a significantly enhanced version of the Ray Tracing assignment (Lab 3). It demonstrates advanced rendering techniques and procedural content generation.

**Features Added:**
1.  **Procedural Textures:** Implemented a checkerboard pattern for the floor plane. This is calculated on-the-fly in the shader based on the world-space intersection point, without requiring external texture files.
    *   **Code Location:** `src/blinn_phong_shading.cpp` (checkerboard logic), `include/Material.h` (flag).
2.  **Complex Material Properties:** The scene features a variety of materials including:
    *   **Mirror Surfaces:** Highly reflective spheres using recursive ray tracing.
    *   **Metallic Surfaces:** Gold-like material with high specular and low diffuse components.
    *   **Matte/Diffuse Surfaces:** Standard Lambertian shading.
3.  **Custom Scene Generation:** Instead of loading a simple JSON file, the scene is constructed programmatically in C++ to create a specific composition of objects and lights that highlights the rendering capabilities.
4.  **Text Overlay:** A custom bitmap font renderer overlays the project title and course information directly onto the final image.

## Acknowledgements
*   **Base Code:** CSC317 Lab 3 (Ray Tracing) starter code.
*   **Libraries:**
    *   `Eigen`: For linear algebra operations.
    *   `json`: For parsing scene files (included in base code).
*   **Font:** A custom 5x7 bitmap font implementation was created for the text overlay.
