#include "../../includes/methods/Get.hpp"
#include "../../includes/methods/CGIhandler.hpp"

Get::Get() : AMethods::AMethods() {}

Get::~Get() {}

//Integrer verification des types de donnees (sert aussi au POST)
//ADD AUTO INDEX ON CONDITION + LOCATION CHECK
//Check parsing URI

void Get::execute(Request& request, Response& response, Server& server)
{
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
	}
}

std::vector<std::string>	Get::serveDirectory(Request& request, Response& response, Server& server)
{
	(void)server;
	struct stat buffer;
	std::vector<std::string> output;
	std::string indexFile = request.getPath() + "index.html";
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
		response.setStatus(200);
	}
	else //ADD AUTO INDEX ON CONDITION + LOCATION CHECK
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
	}
	//ADD AUTO INDEX OFF CONDITION
	return (output);
}