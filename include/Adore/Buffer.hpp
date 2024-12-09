#pragma once
#include "Export.hpp"

#include <Adore/Renderer.hpp>

#include <memory>

namespace Adore
{
    class ADORE_EXPORT Buffer
    {
    protected:
        std::shared_ptr<Renderer> m_renderer;
        Buffer(std::shared_ptr<Renderer>& renderer) : m_renderer(renderer) {};
    public:
        enum class Usage { Vertex, Index };
        static std::shared_ptr<Buffer> create(std::shared_ptr<Renderer>& renderer,
                            Usage const& usage, void* pdata, uint64_t const& size);
        virtual ~Buffer() = default;
        std::shared_ptr<Renderer> renderer() { return m_renderer; }
    };
}