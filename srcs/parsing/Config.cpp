#include "../../includes/parsing/Config.hpp"

Config::Config(const std::string& filename) {
    if (filename.empty())
        throw std::runtime_error("Empty config filename");
        
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + filename);
        
    timestamp("Parsing configuration file: " + filename, YELLOW);
    initParsing(file);
}

Config::~Config()
{
}

Server Config::fillServer(std::vector<std::string>& lines)
{
    Server server;
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        std::cout << "Checking line: [" << *it << "]" << std::endl;
        if ((*it).find("listen") != std::string::npos)
        {
            size_t pos = (*it).find("listen") + 6;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setPort(atoi(value.c_str()));
        }
        else if ((*it).find("server_name") != std::string::npos)
        {
            size_t pos = (*it).find("server_name") + 11;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setHost(value);
        }
        else if ((*it).find("root") != std::string::npos)
        {
            size_t pos = (*it).find("root") + 4;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setRoot(value);
        }
        else if ((*it).find("index") != std::string::npos)
        {
            size_t pos = (*it).find("index") + 5;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setIndex(value);
        }
        else if ((*it).find("client_max_body_size") != std::string::npos)
        {
            size_t pos = (*it).find("client_max_body_size") + 20;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setClientMaxBodySize(value);
        }
        else if ((*it).find("location") != std::string::npos)
        {
            std::cout << "Found location in line: [" << *it << "]" << std::endl;
            size_t pos = (*it).find("location") + 8;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;{");
            std::cout << "Location: " << value << std::endl;
            
            // Maintenant parseLocation avance l'itérateur jusqu'à la fin du bloc
            Location loc = parseLocation(value, lines, it);
            server.pushLocation(loc);
        }
        else if ((*it).find("cgi") != std::string::npos)
        {
            size_t pos = (*it).find("cgi") + 4;
            std::string value = (*it).substr(pos);
            value = trim(value, " \t;");
            server.setCgi(value);
        }
    }
    // DEBUG
    for (size_t i = 0; i < lines.size(); i++) {
            std::cout << "Server Line " << i << ": [" << lines[i] << "]" << std::endl;
        }
    return (server);
}
void Config::parseServer(std::vector<std::string>& lines)
{

    if (lines.empty())
        throw ConfigException("No server found");
    if (lines[0] == "server {")
    {
        std::cout << GREEN << "Server found" << RESET << std::endl;
        Server server;
        std::vector<std::string> serverLines;
        for (std::vector<std::string>::iterator it = lines.begin() + 1; it != lines.end(); it++)
        {
            if ((*it)[0] == '}')
            {
                std::cout << *it << std::endl;
                std::cout << GREEN << "Server closed by '}'" << RESET << std::endl;
                server = fillServer(serverLines);
                std::cout << GREEN << "Server filled" << RESET << std::endl;
                std::cout << server << std::endl;
                addServer(server);
                std::cout << GREEN << "Server added" << RESET << std::endl;

                // Créer un nouveau vecteur avec les lignes restantes
                std::vector<std::string> remainingLines(it + 1, lines.end());
                // Nettoyer les lignes vides au début du vecteur
                while (!remainingLines.empty() && trim(remainingLines[0], WHITESPACES_WITH_SPACE).empty())
                    remainingLines.erase(remainingLines.begin());
                if (!remainingLines.empty()) {
                    parseServer(remainingLines);
                }
                return;
            }
            std::string trimmedLine = trim(*it);
            if (!trimmedLine.empty())
                serverLines.push_back(trimmedLine);
        }
    }
    else
        throw ConfigException("No server found");
}

Location Config::parseLocation(const std::string& location, std::vector<std::string>& lines, std::vector<std::string>::iterator& it)
{
    std::string path = trim(location, " }");
    std::cout << "Path: " << path << std::endl;
    Location NewLocation;
    NewLocation.setPath(path);
    
    // Avancer jusqu'à la fin du bloc de location
    int braceCount = 1; // On commence après l'accolade ouvrante
    it++; // Passer à la ligne suivante après "location xxx {"
    
    while (it != lines.end() && braceCount > 0)
    {
        std::cout << "Line: " << *it << std::endl;
        if ((*it).find("{") != std::string::npos)
            braceCount++;
        if ((*it).find("}") != std::string::npos)
            braceCount--;
        
        if (braceCount == 0)
            break; // On a trouvé l'accolade fermante correspondante
        
        // Traiter les paramètres de la location ici
        // (code pour parser les paramètres comme methods, index, etc.)
        
        it++;
    }
    
    return (NewLocation);
}

void Config::initParsing(std::ifstream& file)
{
    std::string line;
    std::vector<std::string> lines;
    std::cout << GREEN << "/**********PARSING CONFIG FILE**********/" << RESET << std::endl;
    std::cout << CYAN << "----------before parsing----------" << RESET << std::endl;
    while (std::getline(file, line))
    {
        line = trim(line, WHITESPACES);
        if (line.empty() || line[0] == '#')
            continue;
        lines.push_back(line);
    }
    if (!lines.size())
    {
        timestamp("No valid lines found in config file", RED);
        exit(1);
    }
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        std::cout << *it << std::endl;
    }
    std::cout << CYAN << "----------after parsing----------" << RESET << std::endl;
    parseServer(lines);
    std::cout << GREEN << "/**********END OF PARSING**********/" << RESET << std::endl;
    // timestamp("Parsing configuration file: " + filename);
}