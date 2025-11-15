# README #

Technologies: C++, DirectX

Status: Complete, No longer maintained.

A simple demo of different shaders using DX9. It implements a vertex and pixel shader for: phong, goureaud, texture, normal, toon and reflection (cube) mapping.

# What works #
It renders a spinning teapot using different materials, keys 1-8 switch the current effect.

# The objective #
A practice to get the hang of vertex and pixel shaders, this was an assignment for a computer graphics class in my master's degree.

# More information #
This was an exercise on writing vertex and pixel shaders, the basic shaders are implemented, but I added a few variations: normal map, toon shader (with normal map) and a cube reflection to simulate a shiny teapot.

You can find the shader code in the fx file.

# My participation #
I only coded the shader and modified the renderer to allow more effects (and load the correct files for the shader). The bootstrap code and the teapot renderer was provided by the teacher.

# What is broken #
The code was assembled from backups, it may not compile.
