#include "../../includes/methods/Get.hpp"
#include "../../includes/methods/CGIhandler.hpp"

Get::Get() : AMethods::AMethods() {}

Get::Get(const Get& copy) : AMethods::AMethods(copy) {}

Get&	Get::operator=(const Get& copy) {return *this;}

Get::~Get() {}

void Get::execute(Request& request, Response& response, Server& server)
{
	std::string uri = request.getUri();
	FileType file_type = getFileType(uri);
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

bool	Get::checkIfCgi(std::string filepath)
{
	std::vector<std::string>	cgiExt{".php", ".py", ".rb", ".pl"};

	size_t lastDot = filepath.find_last_of('.');
	if (lastDot == std::string::npos)
		return (false);
	std::string	extension = filepath.substr(lastDot);
	for (size_t i = 0; i < cgiExt.size(); i++)
	{
		if (extension == cgiExt[i])
			return (true);
	}
	return (false);
}

void	Get::serveFile(Request& request, Response& response, Server& server)
{
	std::ifstream file(request.getUri());
	if (!file.is_open())
	{
		std::cerr << "Error opening file" << std::endl;
		response.setStatus(404);
		return ;
	}
	if (checkIfCgi(request.getUri()))
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

// bool	Get::isAutoIndex(Server& server)
// {
// 	std::list<
// }

std::vector<std::string>	Get::serveDirectory(Request& request, Response& response, Server& server)
{
	struct stat buffer;
	std::vector<std::string> output;
	std::string indexFile = request.getUri() + "index.html";
	if (stat(indexFile.c_str(), &buffer) == 0) //Is index.html in the directory ?
	{
		std::ifstream file(indexFile);
		if (!file.is_open())
		{
			std::cerr << "Error opening file" << std::endl;
			response.setStatus(404);
			return ;
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
		DIR* current = opendir(request.getUri().c_str());
		struct dirent *ent;
		if (!current)
		{
			std::cerr << "Error opening the directory" << std::endl;
			response.setStatus(404);
			return ;
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