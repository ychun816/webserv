#ifndef UTILS_HPP
#define UTILS_HPP

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define BUFFER_SIZE 4096
#define MAX_REQUEST_SIZE 1024 * 1024 * 10 // 10MB
#include "../../includes/webserv.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../../includes/server/Response.hpp"
#include <signal.h>


extern int g_signal;
void signalHandler(int signum);
void setupSignalHandler();
void cleanupResources();


void timestamp(std::string str, std::string color);
std::string trim(const std::string& str, const std::string& chars = WHITESPACES_WITH_SPACE);
size_t convertSizeToBytes(const std::string& size);
std::string readChunkedData(int client_fd);
size_t hexToSizeT(const std::string& hexStr);
void debugString(Response& response);
#endif
