#include "graphics/Shader.h"

void Shader::load(GraphicsProvider *graphicsProviderPtr, const std::string &fileName)
{
    auto shaderCode = Loader::getInstance().readFile(fileName);
    this->filePath = fileName;
    this->graphicsProviderPtr = graphicsProviderPtr;
    this->module = this->graphicsProviderPtr->createShaderModule(shaderCode);
}

void Shader::destroy()
{
    this->graphicsProviderPtr->destroyShaderModule(this->module);
}