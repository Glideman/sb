#pragma once

#include <vector>

#include "core/Core.h"
#include "primitives/CommonPrimitive.h"

class Grid : public CommonPrimitive
{
protected:
    std::vector<Vertex> getVertexBufferData() override;
    std::vector<uint16_t> getIndexBufferData() override;

public:
    Grid() : CommonPrimitive()
    {
    }
};