#pragma once

#include <Adore/Window.hpp>
#include <Adore/Log.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Initialises GLFW and ensures it is terminated
class GLFWManager
{
public:
    static GLFWManager& instance() { static GLFWManager instance; return instance; }

    GLFWManager() { if (!glfwInit()) throw Adore::AdoreException("Failed to initialise GLFW."); }
    ~GLFWManager() { glfwTerminate(); }

    GLFWManager(const GLFWManager&) = delete;
    GLFWManager& operator=(const GLFWManager&) = delete;
};

class Window : public Adore::Window
{
protected:
    GLFWwindow* m_window;
public:
    Window(std::shared_ptr<Adore::Context>& ctx, std::string const& title)
        : Adore::Window(ctx)
    {
        GLFWManager::instance();

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        int width = mode->width / 2;
        int height = mode->height / 2;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        glfwShowWindow(m_window);
        if (!m_window) throw Adore::AdoreException("Failed to create GLFW window.");
    }

    void resize(int const& width, int const& height) override
    {
        glfwSetWindowSize(m_window, width, height);
    }

    void framebufferSize(uint32_t& width, uint32_t& height) override
    {
        int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
        width = static_cast<uint32_t>(w);
        height = static_cast<uint32_t>(h);
    }
    
    void close() override
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    bool is_open() override
    {
        return !glfwWindowShouldClose(m_window);
    }

    void poll() override { glfwPollEvents(); }

    void surface(VkInstance const& instance, VkSurfaceKHR* pSurface)
    {
        if (glfwCreateWindowSurface(instance, m_window, nullptr, pSurface) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create window surface.");
    }

    static std::vector<char const*> requiredInstanceExtensions()
    {
        GLFWManager::instance();

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        return std::vector<char const*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
    }
};