#include "graphics/Shader.h"
#include "graphics/GraphicsProvider.h"
#include "core/Loader.h"

void Shader::load(const GraphicsProviderPtr graphicsPtr, const std::string &fileName)
{
    this->graphicsPtr = graphicsPtr;
    this->filePath = fileName;

    auto shaderCode = Loader::getInstance().readFile(this->filePath);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    if (vkCreateShaderModule(this->graphicsPtr->getLogicalDevice(), &createInfo, nullptr, &this->shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }
}

void Shader::destroy()
{
    vkDestroyShaderModule(this->graphicsPtr->getLogicalDevice(), this->shaderModule, nullptr);
}

VkShaderModule Shader::getShaderModule()
{
    return this->shaderModule;
}
