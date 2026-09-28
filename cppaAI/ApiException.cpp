#include "ApiException.h"

ApiException::ApiException(const std::string& msg)
{
    message = msg;
}

const char* ApiException::what() const noexcept
{
    return message.c_str();
}