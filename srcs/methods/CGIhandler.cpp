#include "../../includes/methods/CGIhandler.hpp"
#include "../../includes/server/Request.hpp"
#include "../../includes/server/Server.hpp"
#include <stdlib.h>  // Pour exit()
#include <sstream>   // Pour std::ostringstream (conversion int -> string)

// Fonction utilitaire pour remplacer std::to_string
std::string intToString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

CGIhandler::CGIhandler(Request* request, Server* server) : 	_request(request), _server(server)
{
	_scriptPath = resolveScriptPath(request->getUri());
	_interpreter = findInterpreter();
	_postData = "";
	setupEnvironment();
}

CGIhandler::CGIhandler(const CGIhandler& copy) : _request(new Request(*copy._request)), _server(new Server(*copy._server))
{
	_scriptPath = (copy._scriptPath);
	_interpreter = (copy._interpreter);
	_envVars = (copy._envVars);
	_queryString = (copy._queryString);
	_postData = (copy._postData);
}

CGIhandler&	CGIhandler::operator=(const CGIhandler& copy)
{
	if (this != &copy)
	{
		if (_request != NULL) delete _request;
		if (_server != NULL) delete _server;
		_request = new Request(*copy._request);
		_server = new Server(*copy._server);
		_scriptPath = copy._scriptPath;
		_interpreter = copy._interpreter;
		_envVars = copy._envVars;
		_queryString = copy._queryString;
		_postData = copy._postData;
	}
	return *this;
}

CGIhandler::~CGIhandler()
{
	delete _request;
	delete _server;
}

void CGIhandler::setupEnvironment()
{
	_envVars.clear();

	//Informations sur la requête
	_envVars.push_back("REQUEST_METHOD=" + _request->getMethod());
	_envVars.push_back("QUERY_STRING=" + _queryString);
	_envVars.push_back("CONTENT_TYPE=" + _request->getHeader("Content_type"));
	_envVars.push_back("CONTENT_LENGTH=" + _request->getHeader("Content_length"));
	//_envVars.push_back("PATH_INFO=" + sizeof(_request->getBody()));
	//_envVars.push_back("PATH_TRANSLATED=" + sizeof(_request->getBody()));

	//Informations sur le client
	_envVars.push_back("REMOTE_HOST=" + _request->getHeader("Host"));
	_envVars.push_back("HTTP_USER_AGENT=" + _request->getHeader("User-Agent"));
	_envVars.push_back("HTTP_COOKIE=" + _request->getHeader("Cookie"));
	_envVars.push_back("HTTP_ACCEPT=" + _request->getHeader("Accept"));
	_envVars.push_back("HTTP_ACCEPT_LANGUAGE=" + _request->getHeader("Accept-Language"));

	//Informations sur le serveur
	_envVars.push_back("SERVER_NAME=" + _server->getHost());
	_envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_envVars.push_back("SERVER_PORT=" + intToString(_server->getPort()));
	_envVars.push_back("DOCUMENT_ROOT=" + _server->getRoot());

	//Informations sur le script
	_envVars.push_back("SCRIPT_FILENAME=" + _scriptPath);
	_envVars.push_back("SCRIPT_NAME=" + _request->getUri());
}

std::string CGIhandler::resolveScriptPath(const std::string& uri)
{
	return (_server->getRoot() + uri);
}

std::string CGIhandler::findInterpreter()
{
	size_t pos = _scriptPath.find_last_of('.');
	if (pos == std::string::npos)
		return (_scriptPath);
	std::string	extention = _scriptPath.substr(pos);
	if (extention == ".php")
		return ("usr/bin/php");
	else if (extention == ".py")
		return ("usr/bin/python");
	else if (extention == ".pl")
		return ("usr/bin/pearl");
	else if (extention == ".rb")
		return ("usr/bin/ruby");
	return (_scriptPath);
}

std::string CGIhandler::execute()
{
	int inputPipe[2];
	int outputPipe[2];

	if (pipe(inputPipe) || pipe(outputPipe))
		throw std::runtime_error("failed to create pipes");
	int pid = fork();
	if (pid < 0)
		throw std::runtime_error("failed to fork");
	else
	{
		//Manage pipes
		handleRedirection(inputPipe, outputPipe);

		//Prepare args for execve
		char *args[] = {
			(char*)_interpreter.c_str(),
			(char*)_scriptPath.c_str(),
			NULL
		};

		//Convert cgi_env to env
		std::vector<char*> env;
		for (size_t i = 0; i < _envVars.size(); i++)
		{
			env.push_back((char*)_envVars[i].c_str());
		}
		env.push_back(NULL);

		//Exec cgi script
		execve(args[0], args, &env[0]);
		std::cerr << "Failed to exec CGI script" << std::endl;
		exit(1);
	}
	close(inputPipe[0]);
	close(outputPipe[1]);
	// Si c'est une requête POST, écrire les données dans le pipe d'entrée
	if (_request->getMethod() == "POST" && !_postData.empty())
		write(inputPipe[1], _postData.c_str(), _postData.length());
	close(inputPipe[1]);
	// Lire la sortie du script CGI
	char buffer[4096];
	ssize_t bytesRead;
	std::string output;
	while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer) - 1)) > 0)
	{
		buffer[bytesRead] = '\0';
		output += buffer;
	}
	close(outputPipe[0]);
	// Attendre que le processus CGI se termine
	int status;
	waitpid(pid, &status, 0);
	// Vérifier le statut de fin
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		std::cerr << "CGI script exited with status " << WEXITSTATUS(status) << std::endl;
	return output;
}

void CGIhandler::handleRedirection(int inputPipe[2], int outputPipe[2])
{
	close(inputPipe[1]);
	if (dup2(inputPipe[0], STDIN_FILENO) < 0)
	{
		std::cerr << "dup2 for stdin failed" << std::endl;
		exit(1);
	}
	close(inputPipe[0]);

	close(outputPipe[0]);
	if (dup2(outputPipe[0], STDOUT_FILENO) < 0)
	{
		std::cerr << "dup2 for stdout failed" << std::endl;
		exit(1);
	}
	close(outputPipe[1]);
}