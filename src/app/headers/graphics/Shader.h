#pragma once

#include <string>
#include <vector>

#include "core/Loader.h"
#include "graphics/GraphicsProvider.h"

class Shader {
    public:
        std::string name;
        std::string filePath;
        VkShaderModule module;
        GraphicsProvider* graphicsProviderPtr;

        void load(GraphicsProvider* graphicsProviderPtr, const std::string& fileName);
        void destroy();
};