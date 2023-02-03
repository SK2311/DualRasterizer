# DualRasterizer

The dual rasterizer was a project made for my graphics programming class. The goal was to make a rasterizer with 2 modes:
* Software rendering
* Hardware rendering

The software renderer renders the mesh on the CPU. This is not performant at all, but taught us the basics of a rasterizer.

Once the software part was done we started on the hardware. To render the mesh on the GPU, we used DirectX11.

Some things we used to create the rasterizer:
* Depth buffer
* Depth interpolation
* Texture sampling
* DirectX shaders (for the hardware)
* Transparancy (for the hardware)
* OBJ parser
* BRDF

Result software rasterizer:

![alt text](https://github.com/SK2311/DualRasterizer/blob/main/DualRasterizer_Software.JPG)

Result hardware rasterizer:

![alt text](https://github.com/SK2311/DualRasterizer/blob/main/DualRasterizer_DirectX.JPG)

This project was a challenge for me, but I learned so much about how graphics are rendered to the screen.
