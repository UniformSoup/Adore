#pragma once
#include "Export.hpp"

#include <Adore/Renderer.hpp>

#include <memory>

namespace Adore
{
    class Buffer
    {
    public:
        virtual ~Buffer() = default;
        std::shared_ptr<Renderer> renderer() { return m_renderer; }
    protected:
        std::shared_ptr<Renderer> m_renderer;
        Buffer(std::shared_ptr<Renderer>& renderer) : m_renderer(renderer) {};
    };
    
    class ADORE_EXPORT IndexBuffer : public Buffer
    {
    protected:
        IndexBuffer(std::shared_ptr<Renderer>& renderer) : Buffer(renderer) {};
    public:
        static std::shared_ptr<IndexBuffer> create(std::shared_ptr<Renderer>& renderer,
                                              void* pdata, uint64_t const& size);
        virtual ~IndexBuffer() = default;
    };

    class ADORE_EXPORT VertexBuffer : public Buffer
    {
    protected:
        VertexBuffer(std::shared_ptr<Renderer>& renderer) : Buffer(renderer) {};
    public:
        static std::shared_ptr<VertexBuffer> create(std::shared_ptr<Renderer>& renderer,
                                              void* pdata, uint64_t const& size);
        virtual ~VertexBuffer() = default;
    };

    class ADORE_EXPORT UniformBuffer : public Buffer
    {
    protected:
        void * m_pUniformObject;
        uint64_t const m_size;
        UniformBuffer(std::shared_ptr<Renderer>& renderer, void const * pdata, uint64_t const& size)
            : Buffer(renderer), m_size(size)
        {
            m_pUniformObject = malloc(size);
            memcpy(m_pUniformObject, pdata, size);
        };
    public:
        static std::shared_ptr<UniformBuffer> create(std::shared_ptr<Renderer>& renderer,
                                                     void* pdata, uint64_t const& size);
        virtual ~UniformBuffer() { free(m_pUniformObject); };
        uint64_t const& size() { return m_size; }
        void const * get() { return m_pUniformObject; }
        virtual void set(void const * pdata) = 0;
    };

    enum class Filter
    {
        LINEAR,
        NEAREST
    };

    enum class Wrap
    {
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER
    };

    class ADORE_EXPORT Sampler : public Buffer
    {
    protected:
        Sampler(std::shared_ptr<Renderer>& renderer) : Buffer(renderer) {};
    public:
        static std::shared_ptr<Sampler> create(std::shared_ptr<Adore::Renderer>& renderer,
                                               const char* path, Filter const& filter,
                                               Wrap const& wrap);
        virtual ~Sampler() = default;
    };
}