# README #

Technologies: C++, DirectX

Status: Complete, No longer maintained.

A continuation of TeapotDX9, this time a geometry shader is added to simulate 2 effects: one is a "grass" surface made using semitransparent textures, the other is a "hair" simulation used generated geometry.

# What works #
Renders spinning teapots, change technique by using keys 0-9.  

# The objective #
The objective was to learn how to use the geometry shaders to create effects on the GPU.

# More Information #
The grass effect was done by using the geometry shader to create "fins" and "shells". Each fin is a square piece added along each edge with a semitransparent texture of grass blades that when viewed from the side looks like its growing from the edge. The shells consists of rendering a new teapot on an slightly larger scale and using a texture that looks like grass viewed from above (green circles), this was used to show something when the fins were parallel to the eye vector.

The hairs were done by creating extra lines growing from each vertex.

You can view the shader [here](https://github.com/karurosu/3d-teapots/blob/main/d3d10-teapot/D3D10SimpleTeapot/D3D10TeapotTechDiffTexture.fx).

This was also an assignment for my computer graphics class.

# My participation #
The basic renderer and an empty implementation of the grass shader was provided, I wrote the shader code for the effects and modified the C++ code as needed.

# What is broken #
It may be missing files, this was assembled from a backup.