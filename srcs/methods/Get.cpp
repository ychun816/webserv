#include "../../includes/methods/Get.hpp"
#include "../../includes/methods/CGIhandler.hpp"

Get::Get() : AMethods::AMethods() {}

Get::~Get() {}

//Integrer verification des types de donnees (sert aussi au POST)
//Check parsing URI

void Get::execute(Request& request, Response& response, Server& server)
{
	if (!request.validateQueryParams())
	{
		response.setStatus(400);
		response.setBody("<html><body><h1>400 Bad Request: Invalid Query Parameters</h1></body></html>");
		return;
	}
	std::string path = request.getPath();
	FileType file_type = getFileType(path);
	switch (file_type)
	{
		case (TYPE_REGULAR_FILE) :
			serveFile(request, response, server);
			break;
		case (TYPE_DIRECTORY) :
			serveDirectory(request, response, server);
			break;
		case (TYPE_NOT_FOUND) :
			response.setStatus(404);
			response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
			break;
		case (TYPE_NO_PERMISSION) :
			response.setStatus(403);
			response.setBody("<html><body><h1>403 Forbidden</h1></body></html>");
			break;
		default:
			response.setStatus(400);
			response.setBody("<html><body><h1>400 Bad Request</h1></body></html>");
			break;
	}
}

void	Get::serveFile(Request& request, Response& response, Server& server)
{
	// Utilisez .c_str() pour convertir std::string en const char*
	std::ifstream file(request.getPath().c_str());
	if (!file.is_open())
	{
		std::cerr << "Error opening file" << std::endl;
		response.setStatus(404);
		return ;
	}
	if (checkIfCgi(request.getPath()))
	{
		Request* requestPtr = new Request(request);
		Server* serverPtr = new Server(server);
		CGIhandler	execCgi(requestPtr, serverPtr);
		std::string CGIoutput = execCgi.execute();
		response.setBody(CGIoutput);
		response.setStatus(200);
		delete requestPtr;
		delete serverPtr;
	}
	else
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		response.setStatus(200);

		// Définir le type MIME correct
		std::string contentType = getMimeType(request.getPath());
		std::map<std::string, std::string> headers;
		headers["Content-Type"] = contentType;
		response.setHeaders(headers);
	}
}

std::vector<std::string>	Get::serveDirectory(Request& request, Response& response, Server& server)
{
	(void)server;
	struct stat buffer;
	std::vector<std::string> output;
	std::string indexFile = request.getPath() + "index.html";
	std::list<Location>::const_iterator it;
	std::string autoindex = "off";
	for (it = server.getLocations().begin(); it != server.getLocations().end(); ++it) {
		if (it->getPath() == "/")
			autoindex = it->getAutoindex();
	}
	if (stat(indexFile.c_str(), &buffer) == 0) //Is index.html in the directory ?
	{
		// Utilisez .c_str() pour convertir std::string en const char*
		std::ifstream file(indexFile.c_str());
		if (!file.is_open())
		{
			std::cerr << "Error opening file" << std::endl;
			response.setStatus(404);
			return output;
		}
		std::string line;
		while (std::getline(file, line))
		{
			output.push_back(line);
		}
		file.close();
	}
	else if (autoindex == "on")
	{
		// Corriger également cet appel si nécessaire
		DIR* current = opendir(request.getUri().c_str());
		struct dirent *ent;
		if (!current)
		{
			std::cerr << "Error opening the directory" << std::endl;
			response.setStatus(404);
			return output;
		}
		std::vector<std::string> output;
		while ((ent = readdir(current)) != NULL)
		{
			output.push_back(ent->d_name);
		}
		closedir(current);
		std::string directoryListing = "<html><head><title>Directory Listing</title>";
		directoryListing += "<style>body{font-family:Arial,sans-serif;margin:20px;}h1{color:#333;}";
		directoryListing += "ul{list-style-type:none;padding:0;}li{margin:5px 0;padding:5px;border-bottom:1px solid #eee;}";
		directoryListing += "a{text-decoration:none;color:#0066cc;}</style></head>";
		directoryListing += "<body><h1>Directory listing for " + request.getUri() + "</h1><ul>";

		for (size_t i = 0; i < output.size(); i++) {
			directoryListing += "<li><a href=\"" + request.getUri();
			if (request.getUri()[request.getUri().length() - 1] != '/')
				directoryListing += "/";
			directoryListing += output[i] + "\">" + output[i] + "</a></li>";
		}

		directoryListing += "</ul></body></html>";
		response.setBody(directoryListing);
		response.setStatus(200);
	}
	else
	{
		response.setStatus(403);
		response.setBody("<html><body><h1>403 Forbidden: Directory listing not allowed</h1></body></html>");
	}
	return (output);
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