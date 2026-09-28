#pragma once

#include <string>
#include <windows.h>
#include <winhttp.h>
#include <map>
#include "json.hpp"
#include "HttpResponse.h"

using json = nlohmann::json;

class HttpClient 
{
public:
    HttpClient();

    ~HttpClient();

    HttpResponse get(const std::wstring& url);

    void addHeader(
        const std::wstring& key,
        const std::wstring& value
    );

    HttpResponse post(
        const std::wstring& url,
        const std::string& body
    );

    HttpResponse postJson(
        const std::wstring& url,
        const json& data
    );

    std::wstring buildHeaders();

private:
    HINTERNET session;

    std::map<std::wstring, std::wstring> headers;

    bool parseUrl(
        const std::wstring& url,
        std::wstring& host,
        std::wstring& path,
        INTERNET_PORT& port,
        bool& https
    );

    HttpResponse sendRequest(
        const std::wstring& method,
        const std::wstring& url,
        const std::string& body
    );
};