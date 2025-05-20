#ifndef UTILS_HPP
#define UTILS_HPP

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

#include "../../includes/webserv.hpp"

void timestamp(std::string str, std::string color);
std::string trim(const std::string& str, const std::string& chars = WHITESPACES_WITH_SPACE);
size_t convertSizeToBytes(const std::string& size);

#endif
