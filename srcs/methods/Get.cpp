#include "../../includes/methods/Get.hpp"
#include "../../includes/methods/CGIhandler.hpp"
#include "../../includes/utils/Utils.hpp"

Get::Get() : AMethods::AMethods() {}

Get::~Get() {}

//Integrer verification des types de donnees (sert aussi au POST)
//Check parsing URI

void Get::execute(Request& request, Response& response, Server& server)
{
	std::cout << "\n=== DEBUG execute ===" << std::endl;
	std::cout << "Méthode: " << request.getMethod() << std::endl;
	std::cout << "URI: " << request.getUri() << std::endl;
	std::cout << "Chemin absolu: " << request.getAbspath() << std::endl;

	if (!request.validateQueryParams())
	{
		std::cout << "Paramètres de requête invalides" << std::endl;
		response.setStatus(400);
		response.setBody("<html><body><h1>400 Bad Request: Invalid Query Parameters</h1></body></html>");
		return;
	}
	std::string path = request.getAbspath();
	FileType file_type = getFileType(path);
	std::cout << "Type de fichier détecté: " << file_type << std::endl;

	switch (file_type)
	{
		case (TYPE_REGULAR_FILE) :
			std::cout << GREEN << "Traitement comme fichier régulier" << RESET << std::endl;
			serveFile(request, response, server);
			break;
		case (TYPE_DIRECTORY) :
			std::cout << GREEN << "Traitement comme répertoire" << RESET << std::endl;
			serveDirectory(request, response, server);
			break;
		case (TYPE_NOT_FOUND) :
			std::cout << RED << "Fichier non trouvé" << RESET << std::endl;
			response.setStatus(404);
			response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
			break;
		case (TYPE_NO_PERMISSION) :
			std::cout << RED << "Permission refusée" << RESET << std::endl;
			response.setStatus(403);
			response.setBody("<html><body><h1>403 Forbidden</h1></body></html>");
			break;
		default:
			std::cout << RED << "Type de fichier inconnu" << RESET << std::endl;
			response.setStatus(400);
			response.setBody("<html><body><h1>400 Bad Request</h1></body></html>");
			break;
	}
	std::cout << "=== FIN DEBUG execute ===\n" << std::endl;
}

void	Get::serveFile(Request& request, Response& response, Server& server)
{
	std::ifstream file(request.getAbspath().c_str());
	std::cout << request.getAbspath().c_str() << std::endl;
	if (!file.is_open()) {
		std::cerr << RED << "Error opening file" << RESET << std::endl;
		request.fillResponse(response, 404, "<html><body><h1>404 Error: Error opening file</h1></body></html>");
		return;
	}

	// Vérifier la taille du fichier avant de l'envoyer
	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// Vérifier si la taille du fichier dépasse la limite
	size_t maxSize = 10 * 1024 * 1024; // 10MB par défaut
	if (request.getCurrentLocation() && !request.getCurrentLocation()->getClientMaxBodySize().empty()) {
		maxSize = convertSizeToBytes(request.getCurrentLocation()->getClientMaxBodySize());
	} else if (!server.getClientMaxBodySize().empty()) {
		maxSize = convertSizeToBytes(server.getClientMaxBodySize());
	}

	if (fileSize > maxSize) {
		request.fillResponse(response, 413, "<html><body><h1>413 Payload Too Large</h1></body></html>");
		return;
	}

	if (checkIfCgi(request.getAbspath())) {
		std::cout << GREEN << "Exec CGI de script" << RESET << std::endl;
		Request* requestPtr = new Request(request);
		Server* serverPtr = new Server(server);
		CGIhandler execCgi(requestPtr, serverPtr);
		std::string CGIoutput = execCgi.execute();
		request.fillResponse(response, 200, CGIoutput);
		delete requestPtr;
		delete serverPtr;
		std::cout << GREEN << "Fin Exec CGI" << RESET << std::endl;
	} else {
		std::stringstream buffer;
		buffer << file.rdbuf();
		request.fillResponse(response, 200, buffer.str());
		file.close();

		// Définir le type MIME correct
		std::string contentType = getMimeType(request.getAbspath());
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = contentType;
		response.setHeaders(headers);
		std::cout << GREEN << "Fin de traitement fichier régulier" << RESET << std::endl;
	}
}

void Get::serveDirectory(Request& request, Response& response, Server& server)
{
	struct stat buffer;
	std::string indexFile = request.getAbspath();
	std::cout << "indexFile : " << indexFile << std::endl;

	// Déterminer si l'autoindex est activé
	std::list<Location>::const_iterator it = server.getLocations().begin();
	std::string autoindex = it->getAutoindex();
	// std::cout << "Base autoindex : " << autoindex << std::endl;
	// std::cout << "request.getPath() : " << request.getPath() << std::endl;
	// std::cout << "GET-path : " << it->getPath() << std::endl;
	// std::cout << "GET-autoindex : " << it->getAutoindex() << std::endl;
	Location *loc = server.getCurrentLocation(request.getPath());
	if (loc->getAutoindex() != "")
		autoindex = it->getAutoindex();
	std::cout << "GET-autoindex : " << autoindex << std::endl;

	// Vérifier si un fichier index.html existe
	// std::cout << "server : " << server.getHost() << std::endl;
	// std::cout << "stat(indexFile.c_str(), &buffer) : " << stat(indexFile.c_str(), &buffer) << std::endl;
	// std::cout << "autoindex : " << autoindex << std::endl;
	if (stat((indexFile + loc->getIndex()).c_str(), &buffer) == 0)
	{
		std::cout << GREEN << "Index file found" << RESET << std::endl;
		std::ifstream file((indexFile + "index.html").c_str());
		if (!file.is_open())
		{
			std::cerr << "Error opening file" << std::endl;
			request.fillResponse(response, 404, "<html><body><h1>404 Error: Error opening file</h1></body></html>");
			return;
		}

		// Lire le contenu du fichier dans une chaîne
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		// Définir le corps de la réponse avec le contenu du fichier
		request.fillResponse(response, 200, buffer.str());
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "text/html";
		response.setHeaders(headers);
	}
	else if (autoindex == "on")
	{
		std::cout << GREEN << "Autoindex is on" << RESET << std::endl;
		std::string dirPath = "." + server.getRoot() + request.getAbspath();
		DIR* current = opendir(dirPath.c_str());
		struct dirent *ent;

		if (!current)
		{
			std::cerr << "Error opening the directory" << std::endl;
			request.fillResponse(response, 404, "<html><body><h1>403 Forbidden: Directory listing not allowed</h1></body></html>");
			return;
		}

		// Générer la liste des fichiers dans le répertoire
		std::vector<std::string> dirFiles;
		while ((ent = readdir(current)) != NULL)
		{
			dirFiles.push_back(ent->d_name);
		}
		closedir(current);

		// Créer une page HTML pour afficher le contenu du répertoire
		std::string directoryListing = "<html><head><title>Directory Listing</title>";
		directoryListing += "<style>body{font-family:Arial,sans-serif;margin:20px;}h1{color:#333;}";
		directoryListing += "ul{list-style-type:none;padding:0;}li{margin:5px 0;padding:5px;border-bottom:1px solid #eee;}";
		directoryListing += "a{text-decoration:none;color:#0066cc;}</style></head>";
		directoryListing += "<body><h1>Directory listing for " + request.getUri() + "</h1><ul>";

		for (size_t i = 0; i < dirFiles.size(); i++) {
			directoryListing += "<li><a href=\"" + request.getUri();
			if (request.getUri()[request.getUri().length() - 1] != '/')
				directoryListing += "/";
			directoryListing += dirFiles[i] + "\">" + dirFiles[i] + "</a></li>";
		}

		directoryListing += "</ul></body></html>";
		request.fillResponse(response, 200, directoryListing);
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "text/html";
		response.setHeaders(headers);
	}
	else
	{
		std::cout << RED << "Directory listing not allowed" << RESET << std::endl;
		request.fillResponse(response, 403, "<html><body><h1>403 Forbidden: Directory listing not allowed</h1></body></html>");
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = "text/html";
		response.setHeaders(headers);
	}
}

std::string Get::getMimeType(const std::string& path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";  // Type par défaut

	std::string ext = path.substr(dotPos);
	// Convertir l'extension en minuscules
	for (size_t i = 0; i < ext.length(); i++)
		ext[i] = tolower(ext[i]);
	if (ext == ".html" || ext == ".htm")
		return "text/html";
	else if (ext == ".txt")
		return "text/plain";
	else if (ext == ".css")
		return "text/css";
	else if (ext == ".js")
		return "application/javascript";
	else if (ext == ".json")
		return "application/json";
	else if (ext == ".xml")
		return "application/xml";
	else if (ext == ".pdf")
		return "application/pdf";
	else if (ext == ".zip")
		return "application/zip";
	else if (ext == ".jpg" || ext == ".jpeg")
		return "image/jpeg";
	else if (ext == ".png")
		return "image/png";
	else if (ext == ".gif")
		return "image/gif";
	else if (ext == ".svg")
		return "image/svg+xml";
	else if (ext == ".ico")
		return "image/x-icon";
	return "application/octet-stream";  // Type par défaut pour les extensions inconnues
}