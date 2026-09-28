#include "AIClient.h"
#include "ApiException.h"
#include "Config.h"
#include <iostream>


int main()
{

    try
    {
        Config config;
        if (!config.load("config.json"))
        {
            std::cout << "Unable to load config.json\n";
            return 1;
        }

        std::string apiUrl = config.getString("api_url");
        std::string model = config.getString("model");
        if (apiUrl.empty() || model.empty())
        {
            std::cout << "config.json must contain api_url and model\n";
            return 1;
        }
        char* key = nullptr;
        size_t size = 0;

        if (_dupenv_s(&key, &size, "OPENAI_API_KEY") != 0 || key == nullptr)
        {
            std::cout << "API Key不存在\n";
            return 1;
        }

        std::string apiKey(key);
        free(key);

        AIClient ai(
            apiKey,
            std::wstring(apiUrl.begin(), apiUrl.end()),
            model
        );

        std::string answer = ai.ask("解释namespace");

        std::cout << answer;
    }

    catch (ApiException& e)
    {
        std::cout << "API错误:" << e.what();

    }


}
