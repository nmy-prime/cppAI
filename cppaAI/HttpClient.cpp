#include "HttpClient.h"
#include <iostream>
#include <limits>

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

    addHeader(
        L"Content-Type",
        L"application/json"
    );
}

HttpClient::~HttpClient() 
{
    if (session) 
    {
        WinHttpCloseHandle(session);
    }
}

HttpResponse HttpClient::get(
    const std::wstring& url
) 
{
    return sendRequest(
        L"GET",
        url,
        ""
    );
}

void HttpClient::addHeader(
    const std::wstring& key,
    const std::wstring& value
)
{
    headers[key] = value;
}

HttpResponse HttpClient::post(
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

HttpResponse HttpClient::postJson(
    const std::wstring& url,
    const json& data
)
{
    return post(
        url,
        data.dump()
    );
}

std::wstring HttpClient::buildHeaders()
{
    std::wstring result;
    for (auto& header : headers)
    {
        result += header.first;
        result += L": ";
        result += header.second;
        result += L"\r\n";
    }

    return result;
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
    wchar_t extraInfoBuffer[2048];

    components.dwStructSize = sizeof(components);
    components.lpszHostName = hostBuffer;
    components.dwHostNameLength = sizeof(hostBuffer) / sizeof(wchar_t);
    components.lpszUrlPath = pathBuffer;
    components.dwUrlPathLength = sizeof(pathBuffer) / sizeof(wchar_t);
    components.lpszExtraInfo = extraInfoBuffer;
    components.dwExtraInfoLength = sizeof(extraInfoBuffer) / sizeof(wchar_t);

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
    path.append(components.lpszExtraInfo, components.dwExtraInfoLength);

    port = components.nPort;
    https = components.nScheme == INTERNET_SCHEME_HTTPS;

    return true;
}

HttpResponse HttpClient::sendRequest(
    const std::wstring& method,
    const std::wstring& url,
    const std::string& body
)
{
    HttpResponse response;
    std::wstring host;
    std::wstring path;
    INTERNET_PORT port = 0;
    bool https = false;

    if (!session)
    {
        return { 0, "unable to create WinHTTP session" };
    }

    if (body.size() > (std::numeric_limits<DWORD>::max)())
    {
        return { 0, "request body is too large" };
    }

    if (!parseUrl(
        url,
        host,
        path,
        port,
        https
    ))
    {
        return { 0, "invalid url" };
    }

    HINTERNET connection =
        WinHttpConnect(
            session,
            host.c_str(),
            port,
            0
        );

    if (!connection)
    {
        return { 0, "WinHttpConnect failed: " + std::to_string(GetLastError()) };
    }

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
        DWORD error = GetLastError();
        WinHttpCloseHandle(connection);
        return { 0, "WinHttpOpenRequest failed: " + std::to_string(error) };
    }

    std::wstring requestHeaders = buildHeaders();
    DWORD bodySize = static_cast<DWORD>(body.size());

    BOOL sendSuccess = WinHttpSendRequest(
        request,
        requestHeaders.c_str(),
        -1L,
        body.empty()
        ? WINHTTP_NO_REQUEST_DATA
        : (LPVOID)body.data(),
        bodySize,
        bodySize,
        0
    );

    if (!sendSuccess)
    {
        DWORD error = GetLastError();
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);

        return { 0, "WinHttpSendRequest failed: " + std::to_string(error) };
    }

    BOOL receiveSuccess =
        WinHttpReceiveResponse(
            request,
            nullptr
        );

    if (!receiveSuccess)
    {
        DWORD error = GetLastError();
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);

        return { 0, "WinHttpReceiveResponse failed: " + std::to_string(error) };
    }

    //
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

    response.statusCode =static_cast<int>(statusCode);

    std::cout
        << "HTTP Status: "
        << statusCode
        << std::endl;

    // 
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
        response.body.append(
            buffer,
            readSize
        );
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);

    return response;
}
