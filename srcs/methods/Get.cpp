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
		case (TYPE_DIRECTORY) : break;
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

bool	checkIfCgi(std::string filepath)
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
}

void	serveDirectory(std::string uri, Response& response)
{
	response.setStatus(202);
}