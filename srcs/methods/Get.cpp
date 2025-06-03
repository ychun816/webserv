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

	// Vérifier si une redirection est définie pour cette location
	Location* currentLocation = request.getCurrentLocation();
	if (currentLocation && currentLocation->getReturn().first != 0) {
		std::pair<int, std::string> redirectInfo = currentLocation->getReturn();
		int redirectCode = redirectInfo.first;
		std::string redirectUrl = redirectInfo.second;

		// std::cout << "URL de redirection avant envoi: " << redirectUrl << std::endl;  //DEBUG//

		// Créer un corps HTML pour la redirection
		std::string htmlBody = "<html><head><title>Redirection</title>";
		htmlBody += "<meta http-equiv=\"refresh\" content=\"0;url=" + redirectUrl + "\">";
		htmlBody += "</head><body>";
		htmlBody += "<h1>Redirection</h1>";
		htmlBody += "<p>Si vous n'êtes pas redirigé automatiquement, <a href=\"" + redirectUrl + "\">cliquez ici</a>.</p>";
		htmlBody += "</body></html>";

		response.setStatus(redirectCode);
		response.setStatusMessage(response.getStatusMessage(redirectCode));
		std::map<std::string, std::string> headers;
		headers["Location"] = redirectUrl;
		headers["Content-Type"] = "text/html";
		response.setHeaders(headers);
		request.fillResponse(response, redirectCode, htmlBody);
		request.setIsRedirection(true);
		return;
	}
	else if (server.getReturn().first != 0)
	{
		std::pair<int, std::string> redirectInfo = server.getReturn();
		int redirectCode = redirectInfo.first;
		std::string redirectUrl = redirectInfo.second;

		// std::cout << "URL de redirection avant envoi: " << redirectUrl << std::endl;  //DEBUG//

		// Créer un corps HTML pour la redirection
		std::string htmlBody = "<html><head><title>Redirection</title>";
		htmlBody += "<meta http-equiv=\"refresh\" content=\"0;url=" + redirectUrl + "\">";
		htmlBody += "</head><body>";
		htmlBody += "<h1>Redirection</h1>";
		htmlBody += "<p>Si vous n'êtes pas redirigé automatiquement, <a href=\"" + redirectUrl + "\">cliquez ici</a>.</p>";
		htmlBody += "</body></html>";

		response.setStatus(redirectCode);
		response.setStatusMessage(response.getStatusMessage(redirectCode));
		std::map<std::string, std::string> headers;
		headers["Location"] = redirectUrl;
		headers["Content-Type"] = "text/html";
		response.setHeaders(headers);
		request.fillResponse(response, redirectCode, htmlBody);
		request.setIsRedirection(true);
		return;
	}
	if (!request.validateQueryParams())
	{
		std::cout << "Paramètres de requête invalides" << std::endl;
		if (!request.errorPageExist(400)) {
			response.setStatus(400);
			response.setStatusMessage(response.getStatusMessage(400));
			request.buildErrorPageHtml(400, response);
		} else {
			request.openErrorPage(400, response);
		}
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
			if (!request.getHavePriority())
			{
				std::cout << "priority: " << request.getHavePriority() << std::endl;
				if (!request.errorPageExist(404)) {
					response.setStatus(404);
					request.buildErrorPageHtml(404, response);
				} else {
					request.openErrorPage(404, response);
				}
			}
			else
			{
				std::cout << " I dont have priority, so I will not handle this error" << std::endl;
			}
			break;
		case (TYPE_NO_PERMISSION) :
			std::cout << RED << "Permission refusée" << RESET << std::endl;
			if (!request.errorPageExist(403)) {
				response.setStatus(403);
				// std::cout << "🍦🍦HERE IS GET-403TYPE Error page 403 not found, building default error pageNO PERMISSION" << std::endl; //DEBUG//
				request.buildErrorPageHtml(403, response);
			} else {
				request.openErrorPage(403, response);
			}
			break;
		default:
			std::cout << RED << "Type de fichier inconnu" << RESET << std::endl;
			if (!request.errorPageExist(400)) {
				response.setStatus(400);
				request.buildErrorPageHtml(400, response);
			} else {
				request.openErrorPage(400, response);
			}
			break;
	}
	std::cout << "=== FIN DEBUG execute ===\n" << std::endl;
}

void Get::serveFile(Request& request, Response& response, Server& server)
{
	std::string filepath = request.getAbspath();

	// Vérifier d'abord si c'est un script CGI
	if (checkIfCgi(filepath)) {
		std::cout << GREEN << "Exec CGI de script" << RESET << std::endl;
		Request* requestPtr = new Request(request);
		Server* serverPtr = new Server(server);
		CGIhandler execCgi(requestPtr, serverPtr);

		try {
			std::string CGIoutput = execCgi.execute();

			// Séparer les headers et le body de la sortie CGI
			std::string sep = "\r\n";
			size_t headerEnd = CGIoutput.find(sep);
			if (headerEnd != std::string::npos) {
				std::string headers = CGIoutput.substr(0, headerEnd);
				std::string body = CGIoutput.substr(headerEnd + sep.length());

				// Parser les headers CGI
				std::map<std::string, std::string> cgiHeaders;
				std::istringstream headerStream(headers);
				std::string line;

				while (std::getline(headerStream, line) && !line.empty()) {
					if (!line.empty() && line[line.length() - 1] == '\r')
						line = line.substr(0, line.length() - 1);
					size_t colonPos = line.find(':');
					if (colonPos != std::string::npos) {
						std::string key = line.substr(0, colonPos);
						std::string value = line.substr(colonPos + 1);

						// Trim whitespace au début
						while (!value.empty() && value[0] == ' ') {
							value = value.substr(1);
						}

						cgiHeaders[key] = value;
					}
				}
				response.setHeaders(cgiHeaders);
				request.fillResponse(response, 200, body);

			} else {
				// Pas de headers séparés, essayer de détecter si c'est du HTML pur
				if (CGIoutput.find("<html>") != std::string::npos ||
					CGIoutput.find("<!DOCTYPE") != std::string::npos) {
					// C'est du HTML sans headers CGI
					std::map<std::string, std::string> headers;
					headers["Content-Type"] = "text/html";
					response.setHeaders(headers);
					request.fillResponse(response, 200, CGIoutput);
				} else {
					// Traiter comme sortie brute avec headers par défaut
					std::map<std::string, std::string> headers;
					headers["Content-Type"] = "text/plain";
					response.setHeaders(headers);
					request.fillResponse(response, 200, CGIoutput);
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Erreur lors de l'exécution CGI: " << e.what() << std::endl;
			if (!request.errorPageExist(500)) {
				response.setStatus(500);
				request.buildErrorPageHtml(500, response);
			} else {
				request.openErrorPage(500, response);
			}
		}

		//delete requestPtr;
		//delete serverPtr;
		std::cout << GREEN << "Fin Exec CGI" << RESET << std::endl;
		return;
	}

	// Si ce n'est pas un script CGI, traiter comme un fichier statique
	std::ifstream file(filepath.c_str());
	std::cout << filepath.c_str() << std::endl;
	if (!file.is_open()) {
		std::cerr << RED << "Error opening file" << RESET << std::endl;
		if (request.errorPageExist(404)) {
			std::cout << "🍦🍦HERE IS GET-404TYPE Error page 404 not found, open error page" << std::endl;
			request.openErrorPage(404, response);
		} else {
			std::cout << "🍦🍦HERE IS GET-404TYPE Error page 404 not found, building default error page" << std::endl;
			response.setStatus(404);
			request.buildErrorPageHtml(404, response);
		}
		return;
	}

	// Vérifier la taille du fichier avant de l'envoyer
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// Vérifier si la taille du fichier dépasse la limite
	size_t maxSize = 10 * 1024 * 1024; // 10MB par défaut
	if (request.getCurrentLocation() && !request.getCurrentLocation()->getClientMaxBodySize().empty()) {
		maxSize = convertSizeToBytes(request.getCurrentLocation()->getClientMaxBodySize());
	} else if (!server.getClientMaxBodySize().empty()) {
		maxSize = convertSizeToBytes(server.getClientMaxBodySize());
	}

	if (static_cast<size_t>(fileSize) > maxSize) {
		file.close();
		if (!request.errorPageExist(413)) {
			response.setStatus(413);
			request.buildErrorPageHtml(413, response);
		} else {
			request.openErrorPage(413, response);
		}
		return;
	}

	// Lire et servir le fichier statique
	std::stringstream buffer;
	buffer << file.rdbuf();
	request.fillResponse(response, 200, buffer.str());
	file.close();

	// Définir le type MIME correct
	std::string contentType = getMimeType(filepath);
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = contentType;
	response.setHeaders(headers);
	std::cout << GREEN << "Fin de traitement fichier régulier" << RESET << std::endl;
}

void Get::serveDirectory(Request& request, Response& response, Server& server)
{
	struct stat buffer;
	std::string indexFile = request.getAbspath();
	std::cout << "indexFile : " << indexFile << std::endl;

	// Déterminer si l'autoindex est activé

	// std::cout << "Base autoindex : " << autoindex << std::endl;
	// std::cout << "request.getPath() : " << request.getPath() << std::endl;
	// std::cout << "GET-path : " << it->getPath() << std::endl;
	// std::cout << "GET-autoindex : " << it->getAutoindex() << std::endl;
	std::string autoindex = "off";
	if (server.getAutoIndex() != "")
		autoindex = server.getAutoIndex();
	Location *loc = server.getCurrentLocation(request.getPath());
	// if (loc == NULL)
	// {
	// 	std::cerr << "Location not found for path: " << request.getPath() << std::endl;
	// 	if (request.errorPageExist(404)) {
	// 		request.openErrorPage(404, response);
	// 	} else {
	// 		response.setStatus(404);
	// 		response.setStatusMessage(response.getStatusMessage(404));
	// 		request.buildErrorPageHtml(404, response);
	// 	}
	// 	return;
	// }
	if (loc != NULL && loc->getAutoindex() != "")
		autoindex = loc->getAutoindex();
	else
		autoindex = server.getAutoIndex();
	std::cout << "GET-autoindex : " << autoindex << std::endl;

	// Vérifier si un fichier index.html existe
	// std::cout << "server : " << server.getHost() << std::endl;
	// std::cout << "stat(indexFile.c_str(), &buffer) : " << stat(indexFile.c_str(), &buffer) << std::endl;
	// std::cout << "autoindex : " << autoindex << std::endl;
	std::string finalIndex;
	if (loc == NULL || loc->getIndex().empty())
		finalIndex = server.getIndex();
	else
		finalIndex = loc->getIndex();
	if (stat((indexFile + "/" + finalIndex).c_str(), &buffer) == 0 && autoindex != "on")
	{
		std::cout << GREEN << "Index file found" << RESET << std::endl;

		std::cout << "Final index file: " << finalIndex << std::endl;
		std::ifstream file((indexFile + "/" + finalIndex).c_str());
		if (!file.is_open())
		{
			std::cerr << "Error opening file" << std::endl;
			if (request.errorPageExist(404)) {
				// std::cout << "🍦🍦HERE IS GET-404TYPE Error page 404 not found, open error page" << std::endl; //DEBUG//
				request.openErrorPage(404, response);
			} else {
				// std::cout << "🍦🍦HERE IS GET-404TYPE Error page 404 not found, building default error page" << std::endl; //DEBUG//
				response.setStatus(404);
				request.buildErrorPageHtml(404, response);
				// request.fillResponse(response, 404, "<html><body><h1>404 Not Found</h1></body></html>");
			}
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
	else if (autoindex == "on" && !request.getIsRedirection())
	{
		std::cout << GREEN << "Autoindex is on" << RESET << std::endl;
		std::string dirPath = request.getAbspath();
		std::cout << "Directory path: " << dirPath << std::endl;
		DIR* current = opendir(dirPath.c_str());
		struct dirent *ent;

		if (!current)
		{
			std::cerr << "Error opening the directory" << std::endl;
			if (request.errorPageExist(403)) {
				request.openErrorPage(403, response);
			} else {
				response.setStatus(403);
				request.buildErrorPageHtml(403, response);
			}
			return;
		}

		// Générer la liste des fichiers dans le répertoire
		std::vector<std::string> dirFiles;
		while ((ent = readdir(current)) != NULL)
		{
			dirFiles.push_back(ent->d_name);
		}
		closedir(current);

		// Créer une page HTML pour afficher le contenu du répertoire //CHANGED FONT TO BE COHERENT DESIGN
		std::string directoryListing = "<html><head><title>Directory Listing</title>";
		directoryListing += "<head>"
					"<link href=\"https://fonts.googleapis.com/css2?family=Josefin+Sans:wght@300;400;700&display=swap\" rel=\"stylesheet\">"
					"<style>"
					"body{font-family:'Josefin Sans',Arial,sans-serif;margin:20px;}"
					"h1{color:#333;}"
					"ul{list-style-type:none;padding:0;}li{margin:5px 0;padding:5px;border-bottom:1px solid #eee;}"
					"a{text-decoration:none;color:#0066cc;}"
					"</style></head>";
		directoryListing += "<body><h1>Directory listing for " + request.getUri() + "</h1><ul>";

		//print out listing files
		for (size_t i = 2; i < dirFiles.size(); i++)
		{
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
	else if (autoindex == "off")
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