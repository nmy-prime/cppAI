#pragma once

#include <string>
#include "json.hpp"

using json = nlohmann::json;

class JsonParser
{
public:

    static json parse(
        const std::string& text
    );

};