#pragma once

#include <exception>
#include <string>

namespace domain {

struct NeoplasticException : public std::exception {
    explicit NeoplasticException(std::string m) : msg_(std::move(m)) {}
    const char* what() const noexcept override { return msg_.c_str(); }
private:
    std::string msg_;
};

} // namespace domain

