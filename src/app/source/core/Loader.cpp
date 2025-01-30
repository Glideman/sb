#include "core/Loader.h"

void Loader::init(const std::string &dataPath)
{
    this->dataFolders.push_back(dataPath);
}

void Loader::init(const std::vector<std::string> &dataFolders)
{
    for (int i = 0; i < dataFolders.size(); i++)
    {
        this->dataFolders.push_back(dataFolders[i]);
    }
}

std::vector<char> Loader::readFile(const std::string &fileName)
{
    std::vector<char> buffer{};

    for (int i = 0; i < this->dataFolders.size(); i++)
    {
        auto filePath = std::format("{}\\{}", this->dataFolders[i], fileName);
        this->tryToReadFile(filePath, buffer);
        if (!buffer.empty())
            break;
    }

    return buffer;
}

void Loader::tryToReadFile(const std::string &filePath, std::vector<char> &buffer)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (file.is_open())
    {
        size_t fileSize = (size_t)file.tellg();
        buffer.resize(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
    }
}