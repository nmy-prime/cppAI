#include "HttpClient.h"
#include <iostream>


int main()
{
    HttpClient client;
    std::string result =client.get(L"https://example.com");
    std::cout << result;

    return 0;
}