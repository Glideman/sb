#include "core/Loader.h"

void Loader::init(const std::string &dataPath)
{
    this->dataPath = dataPath;
}

std::vector<char> Loader::readFile(const std::string &fileName)
{
    auto filePath = std::format("{}\\{}", this->dataPath, fileName);

    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open file {}", filePath));
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}