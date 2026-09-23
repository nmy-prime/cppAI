#include "HttpClient.h"
#include <iostream>

#pragma comment(lib,"winhttp.lib")

HttpClient::HttpClient() 
{
    session = WinHttpOpen(
        L"CppAI Client",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
}

HttpClient::~HttpClient() 
{
    if (session) 
    {
        WinHttpCloseHandle(session);
    }
}

std::string HttpClient::get(
    const std::wstring& url
) 
{
    return sendRequest(
        L"GET",
        url,
        ""
    );
}

std::string HttpClient::post(
    const std::wstring& url,
    const std::string& body
)
{
    return sendRequest(
        L"POST",
        url,
        body
    );
}

bool HttpClient::parseUrl(
    const std::wstring& url,
    std::wstring& host,
    std::wstring& path,
    INTERNET_PORT& port,
    bool& https
)
{
    URL_COMPONENTS components{};

    wchar_t hostBuffer[256];
    wchar_t pathBuffer[2048];

    components.dwStructSize = sizeof(components);
    components.lpszHostName = hostBuffer;
    components.dwHostNameLength = sizeof(hostBuffer) / sizeof(wchar_t);
    components.lpszUrlPath = pathBuffer;
    components.dwUrlPathLength = sizeof(pathBuffer) / sizeof(wchar_t);

    if (!WinHttpCrackUrl(url.c_str(), 
        0, 
        0, 
        &components)
        )
    {
        return false;
    }
    host.assign(
        components.lpszHostName,
        components.dwHostNameLength
    );

    path.assign(
        components.lpszUrlPath,
        components.dwUrlPathLength
    );

    port = components.nPort;
    https = components.nScheme == INTERNET_SCHEME_HTTPS;

    return true;
}

std::string HttpClient::sendRequest(
    const std::wstring& method,
    const std::wstring& url,
    const std::string& body
)
{
    std::string response;
    std::wstring host;
    std::wstring path;
    INTERNET_PORT port;
    bool https;

    if (!parseUrl(
        url,
        host,
        path,
        port,
        https
    ))
    {
        return "invalid url";
    }

    HINTERNET connection =
        WinHttpConnect(
            session,
            host.c_str(),
            port,
            0
        );

    HINTERNET request = WinHttpOpenRequest(
            connection,
            method.c_str(),
            path.c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            https ?
            WINHTTP_FLAG_SECURE :
            0
        );

    if (!request)
    {
        WinHttpCloseHandle(connection);
        return "request failed";
    }

    LPCWSTR headers = L"Content-Type: application/json\r\n";

    BOOL sendSuccess = WinHttpSendRequest(
        request,
        headers,
        -1L,
        body.empty()
        ? WINHTTP_NO_REQUEST_DATA
        : (LPVOID)body.data(),
        body.size(),
        body.size(),
        0
    );

    if (!sendSuccess)
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);

        return "send failed";
    }

    BOOL receiveSuccess =
        WinHttpReceiveResponse(
            request,
            nullptr
        );

    if (!receiveSuccess)
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);

        return "receive failed";
    }

    // 获取状态码

    DWORD statusCode = 0;
    DWORD statusSize =sizeof(statusCode);

    WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_STATUS_CODE |
        WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusSize,
        WINHTTP_NO_HEADER_INDEX
    );

    std::cout
        << "HTTP Status: "
        << statusCode
        << std::endl;
    // 读取数据

    DWORD size = 0;

    while (
        WinHttpQueryDataAvailable(request, &size) && size > 0
        )
    {
        char buffer[1024];
        DWORD readSize = 0;
        BOOL readSuccess =
            WinHttpReadData(
                request,
                buffer,
                sizeof(buffer),
                &readSize
            );

        if (!readSuccess)
        {
            break;
        }

        response.append(
            buffer,
            readSize
        );
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);

    return response;
}