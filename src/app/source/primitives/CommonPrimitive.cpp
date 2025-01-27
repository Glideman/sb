#include "primitives/CommonPrimitive.h"
#include "graphics/Mesh.h"

void CommonPrimitive::create(const GraphicsProviderPtr graphicsPtr)
{
    this->mesh = new Mesh();

    std::vector<Vertex> vertices = this->getVertexBufferData();
    std::vector<uint16_t> indices = this->getIndexBufferData();

    this->mesh->create(graphicsPtr, vertices, indices);
}

void CommonPrimitive::destroy()
{
    this->mesh->destroy();
    delete this->mesh;
    this->mesh = nullptr;
}

std::vector<Vertex> CommonPrimitive::getVertexBufferData()
{
    return std::vector<Vertex>{};
}

std::vector<uint16_t> CommonPrimitive::getIndexBufferData()
{
    return std::vector<uint16_t>{};
}

MeshPtr CommonPrimitive::getMesh()
{
    return this->mesh;
}
