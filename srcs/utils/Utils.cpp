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

size_t convertSizeToBytes(const std::string& size) {
    size_t multiplier = 1;
    size_t value = 0;
    
    if (size.empty()) return 0;
    
    // Extract the numeric value
    size_t i = 0;
    while (i < size.length() && isdigit(size[i])) {
        value = value * 10 + (size[i] - '0');
        i++;
    }
    
    // Suffix handling
    if (i < size.length()) {
        char suffix = toupper(size[i]);
        switch (suffix) {
            case 'K': multiplier = 1024; break;
            case 'M': multiplier = 1024 * 1024; break;
            case 'G': multiplier = 1024 * 1024 * 1024; break;
            default: return 0;
        }
    }
    
    return value * multiplier;
}

