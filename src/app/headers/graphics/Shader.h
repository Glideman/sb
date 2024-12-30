#pragma once

#include "core/Core.h"

#include <string>
#include <vector>

#include "core/Vulkan.h"

class Shader
{
private:
    std::string name;
    std::string filePath;
    VkShaderModule shaderModule;
    GraphicsProviderPtr graphicsPtr;

public:
    Shader()
    {
        this->name = "";
        this->filePath = "";
        this->shaderModule = nullptr;
        this->graphicsPtr = nullptr;
    };

    void load(const GraphicsProviderPtr graphicsPtr, const std::string &fileName);
    void destroy();

    VkShaderModule getShaderModule();
};