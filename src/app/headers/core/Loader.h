#pragma once

#include "core/Core.h"

#include <string>
#include <vector>
#include <format>
#include <fstream>

class Loader
{
private:
    Loader()
    {
        this->dataFolders.resize(0);
    }

    std::vector<std::string> dataFolders;

    void tryToReadFile(const std::string &filePath, std::vector<char> &buffer);

public:
    Loader(Loader const &) = delete;
    void operator=(Loader const &) = delete;

    static Loader &getInstance()
    {
        static Loader pInstance;
        return pInstance;
    }

    void init(const std::string &dataPath);
    void init(const std::vector<std::string> &dataFolders);
    std::vector<char> readFile(const std::string &fileName);
};