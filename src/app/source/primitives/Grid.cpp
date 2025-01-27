#include "primitives/Grid.h"

std::vector<Vertex> Grid::getVertexBufferData()
{
    std::vector<Vertex> vertices;

    float width = 5.f, height = 5.f;
    uint8_t xSections = 10, ySections = 10;

    float xStart = 0 - (width / 2);
    float yStart = 0 - (height / 2);

    for (int y = 0; y < ySections; y++)
    {
        for (int x = 0; x < xSections; x++)
        {
            int colorComponent = (x + y) % 2;
            float colorGradient = 0.3f + (float)colorComponent * 0.3f;

            float x0 = xStart + width / xSections * x;
            float y0 = yStart + height / ySections * y;
            float x1 = xStart + width / xSections * (x + 1);
            float y1 = yStart + height / ySections * (y + 1);

            Vertex vert1 = {{x0, y0, 0.f}, {colorGradient, colorGradient, colorGradient}};
            Vertex vert2 = {{x1, y1, 0.f}, {colorGradient, colorGradient, colorGradient}};
            Vertex vert3 = {{x0, y1, 0.f}, {colorGradient, colorGradient, colorGradient}};
            Vertex vert4 = {{x1, y0, 0.f}, {colorGradient, colorGradient, colorGradient}};

            vertices.push_back(vert1);
            vertices.push_back(vert2);
            vertices.push_back(vert3);
            vertices.push_back(vert4);
        }
    }

    return vertices;
}

std::vector<uint16_t> Grid::getIndexBufferData()
{
    std::vector<uint16_t> indices;

    float width = 5.f, height = 5.f;
    uint8_t xSections = 10, ySections = 10;

    float xStart = 0 - (width / 2);
    float yStart = 0 - (height / 2);

    for (int y = 0; y < ySections; y++)
    {
        for (int x = 0; x < xSections; x++)
        {
            int indexComponent = (x + y * xSections) * 4;

            indices.push_back(indexComponent + 0);
            indices.push_back(indexComponent + 1);
            indices.push_back(indexComponent + 2);
            indices.push_back(indexComponent + 0);
            indices.push_back(indexComponent + 3);
            indices.push_back(indexComponent + 1);
        }
    }

    return indices;
}