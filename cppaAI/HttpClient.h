#pragma once

#include <string>
#include <windows.h>
#include <winhttp.h>

class HttpClient 
{
public:
    HttpClient();

    ~HttpClient();

    std::string get(
        const std::wstring& url
    );

    std::string post(
        const std::wstring& url,
        const std::string& body
    );

private:
    HINTERNET session;

    bool parseUrl(
        const std::wstring& url,
        std::wstring& host,
        std::wstring& path,
        INTERNET_PORT& port,
        bool& https
    );

    std::string sendRequest(
        const std::wstring& method,
        const std::wstring& url,
        const std::string& body
    );

};