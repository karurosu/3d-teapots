# README #

Technologies: C++, DirectX

Status: Complete, No longer maintained.

The last project in the Teapot series, this uses a very simplistic phong model and a tessellation shader to render a (very) smooth teapot.

# What works #
Renders a moving teapot using a tessellation shader, thats about it.

# The objective #
Learn DX11 by using tessellation, specifically how to use a hull and tessellation shaders.

# More Information #
It simply renders some teapots using constant tessellation. You can find the code [here](https://bitbucket.org/karurosu/teapotdx11/src/64e6a211db8ded965309817ffbb65db92ca00ec1/D3D11SimpleTeapot/D3D11SimpleFx.hlsl?at=default&fileviewer=file-view-default). The tessellation was done using a bezier algorithm and control points.

Shading was kept simple: a goureaud without texture, the math for the bezier curves was implemented in the shader using matrix multiplications.

This was the final assignment for my computer graphics class.

# My participation #
In this case I implemented the mathematical transformation (based on the theory), but all the supporting DX11 code was given to us. I cannot claim more credit than that.

# What is broken #
It may not compile, this repo was assembled from a backup.