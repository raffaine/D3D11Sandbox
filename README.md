D3D11Sandbox for Win32
==================

D3D11Sandbox is as the name imply a place for noobies such as myself to play freely.

Ambition is for the skills and components built here to become useful and be part of a game I've been thinking about.

Right now I want to use it for experimentation with the API, which if I'm not mistaken is gracefully turning 10 years old.

The current state I have a running Win32 app, that splits its window into a Canvas area and a Settings area, where I can configure settings for the canvas. As soon as it's required I will add Keyboard, Mouse and Touch Interactions to the Canvas, but currently it's display only.

The Scene loaded is of a colored spinning cube, which has adjustable number of vertices (for a surprising effect). I have a basic set of shaders there that paint it, and do some basic lighting so that the mesh resolution is more noticeable. Also, I'm adding mesh loading with initial support for GLTF ([spec](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#introduction)), this requires JSON parsing which is done using a Nuget Package ([nlohmann json](https://github.com/nlohmann/json)). Make sure packages are loaded before trying to compile. I've added an extremely simple model I made with Blender (lol ... didn't know that the mirroring was only on Blender) and it's displaying fine.
