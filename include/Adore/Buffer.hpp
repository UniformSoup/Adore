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
}