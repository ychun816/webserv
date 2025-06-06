#include "../../includes/webserv.hpp"
#include "../../includes/utils/Utils.hpp"
#include "../../includes/parsing/Config.hpp"
#include "../../includes/server/EpollManager.hpp"

int g_signal = 0;


std::string readChunkedData(int client_fd) {
    std::string completeData;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    size_t totalBytesRead = 0;
    int chunk_count = 0;

    std::cout << "\n=== DEBUT LECTURE CHUNKS ===" << std::endl;
    std::cout << "Client " << client_fd << " - Début lecture" << std::endl;

    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        chunk_count++;
        completeData.append(buffer, bytes_read);
        totalBytesRead += bytes_read;

        std::cout << "CHUNK #" << chunk_count
                  << " | Taille: " << bytes_read
                  << " bytes | Total: " << totalBytesRead 
                  << " bytes" << std::endl;

        if (totalBytesRead > MAX_REQUEST_SIZE) {
            std::cerr << "ERREUR: Requête trop grande: " << totalBytesRead << " bytes" << std::endl;
            throw std::runtime_error("Request too large");
        }
    }

    std::cout << "=== FIN LECTURE CHUNKS ===" << std::endl;
    std::cout << "Client " << client_fd
              << " | Chunks: " << chunk_count
              << " | Taille totale: " << totalBytesRead
              << " bytes\n" << std::endl;

    return completeData;
}

std::string trim(const std::string& str, const std::string& chars) {
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

size_t hexToSizeT(const std::string& hexStr) {
    size_t result = 0;
    for (size_t i = 0; i < hexStr.length(); ++i) {
        char c = hexStr[i];
        result *= 16;
        if (c >= '0' && c <= '9') {
            result += c - '0';
        } else if (c >= 'a' && c <= 'f') {
            result += c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            result += c - 'A' + 10;
        }
    }
    return result;
}

void debugString(int code)
{
    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    std::string dateTime = buffer;

    // std::cout << "Code de réponse : " << code << std::endl;
    std::string color;
    if (code >= 200 && code < 300)
        color = "\033[1;32m"; 
    else if (code >= 300 && code < 400)
        color = "\033[1;36m"; 
    else if (code >= 400 && code < 500)
        color = "\033[1;33m"; 
    else if (code >= 500)
        color = "\033[1;31m"; 
    else
        color = "\033[0m";    

    std::cout << color << "[" << dateTime << "] [Response] Code: " << code << "\033[0m" << std::endl;
}

void signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        std::cout << "\n" << RED << "Signal reçu: " << signum << RESET << std::endl;
        g_signal = signum;
        cleanupResources();
        exit(0);
    }
}
void setupSignalHandler() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << RED << "Échec de l'installation du gestionnaire pour SIGINT" << RESET << std::endl;
        exit(EXIT_FAILURE);
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        std::cerr << RED << "Échec de l'installation du gestionnaire pour SIGTERM" << RESET << std::endl;
        exit(EXIT_FAILURE);
    }
}


void cleanupResources() {
    // Freeing resources, closing sockets
    std::cout << "Nettoyage des ressources..." << std::endl;
    Config* config = Config::getInstance();
    EpollManager* epollManager = EpollManager::getInstance();
    if (epollManager) {
        std::cout << "Fermeture de l'instance EpollManager..." << std::endl;
        delete epollManager;
    }
    if (config) {
        std::cout << "Arrêt des serveurs..." << std::endl;
        config->stopServers();
        delete config; 
    }
}