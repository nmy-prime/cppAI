#pragma once

#include <exception>
#include <string>

class ApiException :
    public std::exception

{
public:
    ApiException(const std::string& message);

    const char* what() const noexcept override;

private:
    std::string message;
};