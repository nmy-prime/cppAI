#pragma once

#include <string>
#include "json.hpp"

using json = nlohmann::json;

class Config
{
public:
    bool load(const std::string& filename);

    std::string getString(const std::string& key);

private:
    json data;
};