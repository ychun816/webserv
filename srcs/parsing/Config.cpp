#include "../../includes/parsing/Config.hpp"
#include <sstream>
#include <sys/wait.h> // Pour waitpid

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
Location Config::parseLocation(const std::string& location, std::vector<std::string>& lines, std::vector<std::string>::iterator& it)
{
	// std::string path = trim(location, " }");
	std::cout << "Path: " << location << std::endl;
	Location NewLocation;
	NewLocation.setPath(location);
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

		// Traiter les paramètres de la location
		if ((*it).find("methods") != std::string::npos)
		{
			size_t pos = (*it).find("methods") + 7;
			std::string value = (*it).substr(pos);
			std::vector<std::string> methodsVector;
			value = trim(value, " \t;");
			std::stringstream ss(value);
			std::string method;
			while (ss >> method) {
				methodsVector.push_back(method);
			}
			NewLocation.setMethods(methodsVector);
		}
		else if ((*it).find("index") != std::string::npos)
		{
			size_t pos = (*it).find("index") + 5;
			std::string value = (*it).substr(pos);
			value = trim(value, " \t;");
			NewLocation.setIndex(value);
		}
		else if ((*it).find("root") != std::string::npos)
		{
			size_t pos = (*it).find("root") + 4;
			std::string value = (*it).substr(pos);
			value = trim(value, " \t;");
			NewLocation.setRoot(value);
		}
		else if ((*it).find("cgi_ext") != std::string::npos)
		{
			size_t pos = (*it).find("cgi_ext") + 7;
			std::string value = (*it).substr(pos);
			value = trim(value, " \t;");
			NewLocation.setCgiExt(value);
		}
		else if ((*it).find("client_max_body_size") != std::string::npos)
		{
			size_t pos = (*it).find("client_max_body_size") + 20;
			std::string value = (*it).substr(pos);
			value = trim(value, " \t;");
			NewLocation.setClientMaxBodySize(value);
		}
		else if ((*it).find("autoindex") != std::string::npos)
		{
			size_t pos = (*it).find("autoindex") + 9;
			std::string value = (*it).substr(pos);
			value = trim(value, " \t;");
			NewLocation.setAutoindex(value);
		}
		else if ((*it).find("upload_path") != std::string::npos)
		{
			size_t pos = (*it).find("upload_path") + 11;
			std::string value = (*it).substr(pos);
			value = trim(value, " \t;");
			NewLocation.setUploadPath(value);
		}
		it++;
	}

	return (NewLocation);
}

void Config::findParameters(std::vector<std::string>::iterator& it, Server& server, std::vector<std::string>& lines)
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
		else if ((*it).find("methods") != std::string::npos)
		{
			size_t pos = (*it).find("methods") + 7;
			std::string value = (*it).substr(pos);
			value = trim(value, " \t;");
			std::list<std::string> methods;
			std::stringstream ss(value);
			std::string method;
			while (ss >> method) {
				methods.push_back(method);
			}
			server.setAllowMethods(methods);
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

			// parseLocation avance l'itérateur jusqu'à la fin du bloc puis on push la location dans le server
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

Server Config::fillServer(std::vector<std::string>& lines)
{
	Server server;
	for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
	{
		findParameters(it, server, lines);
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

	// Vérifier si la première ligne est un début de bloc serveur
	if (lines[0] == "server {")
	{
		std::cout << GREEN << "Server found" << RESET << std::endl;
		Server server;
		std::vector<std::string> serverLines;
		int braceCount = 1; // Compteur d'accolades pour gérer les blocs imbriqués

		std::vector<std::string>::iterator it = lines.begin() + 1;
		for (; it != lines.end(); ++it)
		{
			// Compter les accolades pour s'assurer de trouver la fin du bloc serveur actuel
			if ((*it).find("{") != std::string::npos) braceCount++;
			if ((*it).find("}") != std::string::npos) braceCount--;

			// Si on trouve la fin du bloc serveur actuel
			if (braceCount == 0)
			{
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

				// S'il reste des lignes, continuer le parsing pour trouver d'autres serveurs
				if (!remainingLines.empty()) {
					parseServer(remainingLines);
				}
				return;
			}

			// Ajouter la ligne au serveur actuel
			std::string trimmedLine = trim(*it);
			if (!trimmedLine.empty())
				serverLines.push_back(trimmedLine);
		}

		// Si on arrive ici, c'est qu'on n'a pas trouvé la fin du bloc serveur
		if (braceCount > 0)
			throw ConfigException("Unclosed server block");
	}
	else
		throw ConfigException("No server found");
}

void Config::runServers()
{
	if (_servers.empty()) {
		std::cerr << "Aucun serveur à démarrer" << std::endl;
		return;
	}

	std::cout << "Démarrage de " << _servers.size() << " serveurs..." << std::endl;

	// Pour lancer tous les serveurs, on utilise fork() pour créer des processus séparés
	for (size_t i = 0; i < _servers.size(); i++)
	{
		pid_t pid = fork();

		if (pid == 0) {
			// Processus enfant
			std::cout << "Démarrage du serveur " << i << " sur le port " << _servers[i].getPort() << " (processus " << getpid() << ")" << std::endl;
			_servers[i].createSocket();
			_servers[i].configSocket();
			_servers[i].runServer();
			exit(0); // Terminer le processus enfant après la fin du serveur
		}
		else if (pid < 0) {
			// Erreur de fork
			std::cerr << "Erreur lors de la création du processus pour le serveur " << i << std::endl;
		}
		else {
			// Processus parent
			std::cout << "Serveur " << i << " lancé dans le processus " << pid << std::endl;
		}
	}

	// En mode multi-processus, le processus parent attend que tous les enfants se terminent
	// Ce qui n'arrivera jamais sauf en cas d'erreur ou de signal
	int status;
	waitpid(-1, &status, 0); // Attendre n'importe quel enfant
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
	// Ne pas démarrer automatiquement les serveurs à la fin du parsing
	// runServers();
	// timestamp("Parsing configuration file: " + filename);
}