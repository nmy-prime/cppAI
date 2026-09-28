#include "Config.h"

#include <fstream>

bool Config::load(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    try
    {
        file >> data;
    }
    catch (const json::exception&)
    {
        return false;
    }

    return true;
}

std::string Config::getString(const std::string& key)
{
    if (!data.contains(key))
    {
        return "";
    }
    if (!data[key].is_string())
    {
        return "";
    }

    return data[key].get<std::string>();
}
