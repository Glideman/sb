#pragma once

#include <string>
#include <vector>
#include <format>
#include <fstream>

class Loader
{
private:
    Loader()
    {
        this->dataPath = "";
    }

    std::string dataPath;

public:
    Loader(Loader const &) = delete;
    void operator=(Loader const &) = delete;

    static Loader &getInstance()
    {
        static Loader pInstance;
        return pInstance;
    }

    void init(const std::string &dataPath);
    std::vector<char> readFile(const std::string &fileName);
};