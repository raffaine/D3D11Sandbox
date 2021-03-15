D3D11Sandbox is as the name imply a place for noobies such as myself to play freely.

Ambition is for the skills and components built here to become useful and be part of a game I've been thinking about.

Right now I want to use it for experimentation with the API.

The current state I have a running Win32 app, that splits its window into a Canvas area and a Settings area, where I can configure settings for the canvas. As soon as it's required,
I will add Keyboard, Mouse and Touch Interactions to the Canvas, but currently it's display only.

The Scene loaded is of a colored spinning cube, which has adjustable number of vertices (currently unnoticeable, but I've debugged it and I promisse they are there). I have a basic
set of shaders there that paint it, and will soon add some lighting so that the mesh resolution is more noticeable. Also, this will become a Sphere, and will be my first on the Mesh
toolkit I want to have available to toy with before going onto exporters and mesh loading.
