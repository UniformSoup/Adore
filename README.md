# <center>Adore Engine</center>

Total lines of C++ code: 1210

Total lines of CMake code: 256

## <center>TODO:</center>

Give the shader bind functions for textures and uniform buffers. It should also hold the textures and buffers so that they can be re bound if one changes and are also not destroyed while in use. 
I need to store some info which describes the bind point and buffer / texture and then calls vkUpdateDescriptorSets.

Next:
 - Textures
 - Meshes / Models
 - Depth Buffering
 - Mipmapping
 - Multisampling

 - Animations

 - Application (what structure?? Layers? State?)
 - Input (Input Manager + Events)

 - Settings object / customise window vsync presentmode etc.
   (ie big settings struct with a window settings, sound settings ect)

 - UI
    - Fonts
    - Text

 - Audio

 - Use AMD's vulkan memory allocator (VMA) instead of manual memory allocation.

 - c_vars
 - Profiling
 - Physics (Box2D? Bullet?)
 - ECS (maybe SECS or EnTT)

 - Implement instance buffer if I can ever grow gonads.

 - RENDERER TAKES WINDOW, SHADER TAKES WINDOW HOW TO BE SURE THE WINDOW IS THE SAME?
   - They only share a renderpass? do I need to check? need more experience with vulkan.
     (Currently checking m_window() == shader.window())

- Supporting multiple windows with shared resources cannot be done until the device
  becomes part of the context. REALLY need to figure out a way to have the window only
  contain a surface and swapchain, so multiple windows can share buffers and renderer.
  The issue is that to query a queue index for present support, a surface must be created,
  so queue / device creation, must come after window creation. Unless I circumvent this in the future.

  https://www.reddit.com/r/vulkan/comments/p9whak/sharing_vertex_buffers_amongst_multiple_windows/
  "The only object that needs to be unique per window is the swapchain (and thus its images), it's up to you how you render on it."
  Maybe I misunderstood the concept of a surface... I thought it was a window handle.
  Okay I am correct, but I could probably cheese this and make a temp surface to query device stuff? then all is balling?

  **Why not have the context contain the device, but initialise it when the first window is created?
  eg call a function to initialise the device if m_device == VK_NULL_HANDLE using the window surface.**

- Uniforms may try to write while the buffer is being used by the gpu
  m_currentFrame needs to be protected by the thread somehow
  possibly a whole structure change.

- Swapchain queue exclusivity problem.

- Context should query max anisotropy, and take a anisotropy parameter in the sampler.