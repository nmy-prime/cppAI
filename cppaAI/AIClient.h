#pragma once

#include "HttpClient.h"
#include <string>

class AIClient
{
public:
    AIClient(
        std::string apiKey,
        std::wstring url,
        std::string model
    );
    std::string ask(std::string question);

private:
    HttpClient http;

    std::wstring apiUrl;
    std::string model;
    std::string apiKey;

    std::string handleHttpError(int code);
};