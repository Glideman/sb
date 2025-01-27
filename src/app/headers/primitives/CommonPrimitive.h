#pragma once

#include <vector>

#include "core/Core.h"
#include "graphics/Vertex.h"

class CommonPrimitive
{
protected:
    MeshPtr mesh;

    virtual std::vector<Vertex> getVertexBufferData();
    virtual std::vector<uint16_t> getIndexBufferData();

public:
    CommonPrimitive()
    {
        this->mesh = nullptr;
    }

    virtual void create(const GraphicsProviderPtr graphicsPtr);
    virtual void destroy();

    MeshPtr getMesh();
};