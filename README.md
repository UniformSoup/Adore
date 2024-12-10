# <center>Adore Engine</center>

Total lines of C++ code: 1210

Total lines of CMake code: 256

## <center>TODO:</center>

Next:
 - Textures

  - Application (what structure?? Layers? State?)
  - Input (Input Manager + Events)

 - Settings object / customise window vsync presentmode etc.
   (ie big settings struct with a window settings, sound settings ect)

 - UI
    - Fonts
    - Text

 - Profiling

 - Meshes / Models
 - Animations

 - Use AMD's vulkan memory allocator (VMA) instead of manual memory allocation.

 - Physics (Box2D? Bullet?)
 - c_vars
 - ECS (maybe SECS or EnTT)

 - RENDERER TAKES WINDOW, SHADER TAKES WINDOW HOW TO BE SURE THE WINDOW IS THE SAME?
   - They only share a renderpass? do I need to check? need more experience with vulkan.
     (Currently checking m_window() == shader.window())