#include "AIClient.h"
#include "ApiException.h"


AIClient::AIClient(
    std::string key,
    std::wstring url,
    std::string model
)
{
    apiKey = key;
    apiUrl = url;
    this->model = model;

    http.addHeader(L"Authorization", L"Bearer " +
        std::wstring(
            key.begin(),
            key.end()
        )
    );

    http.addHeader(
        L"Content-Type",
        L"application/json"
    );
}

std::string AIClient::ask(std::string question)
{
    json request;
    request["messages"] =
    {
        {
            {
                "role",
                "user"
            },
            {
                "content",
                question
            }
        }
    };
    request["model"] = model;

    HttpResponse response =
        http.postJson(apiUrl, request);

    if (response.statusCode != 200)
    {
        throw ApiException(
            response.body.empty()
                ? handleHttpError(response.statusCode)
                : response.body
        );
    }

    json result;

    try
    {
        result = json::parse(response.body);
    }
    catch (const json::exception&)
    {
        throw ApiException("JSON Parse Error");
    }

    if (!result.contains("choices") ||
        !result["choices"].is_array() ||
        result["choices"].empty() ||
        !result["choices"][0].contains("message") ||
        !result["choices"][0]["message"].contains("content") ||
        !result["choices"][0]["message"]["content"].is_string())
    {
        throw ApiException("Response missing answer");
    }

    return result["choices"][0]["message"]["content"];
}

std::string AIClient::handleHttpError(int code)
{

    switch (code)
    {
    case 401:
        return "Unauthorized";
    case 404:
        return "Not Found";
    case 429:
        return "Too Many Requests";
    case 500:
        return "Server Error";

    default:
        return "HTTP Error:" + std::to_string(code);
    }

}
