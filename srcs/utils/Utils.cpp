#include "../../includes/webserv.hpp"

std::string trim(const std::string& str, const std::string& chars = WHITESPACES_WITH_SPACE) {
    if (str.empty()) return "";

    size_t first = str.find_first_not_of(chars);
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(chars);
    return str.substr(first, last - first + 1);
}


void timestamp(std::string str, std::string color)
{
    if (str.empty())
        return;
    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    std::string dateTime = buffer;
    std::cout << color << "[" << dateTime << "] " << str << RESET << std::endl;
}