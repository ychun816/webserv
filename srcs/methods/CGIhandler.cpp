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
	//std::cout <<  "_scriptPath === " << _scriptPath << std::endl;
	_interpreter = findInterpreter();
	//std::cout <<  "_interpreter === " << _interpreter << std::endl;
	if (request->getMethod() == "POST") {
		_postData = request->getBody();  // ← Get the actual POST data
		//std::cout << "POST data populated: " << _postData << std::endl;  // Debug
	} else {
		_postData = "";
	}
	setupEnvironment();
	// std::cout <<  "===== ENV VARS =====" << std::endl;
	// std::vector<std::string>::iterator it = _envVars.begin();
	// for (; it != _envVars.end(); it++)
	// {
	// 	std::cout << *it << std::endl;
	// }
	// std::cout <<  "===== ENV VARS END =====" << std::endl;
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

	//Request information
	_envVars.push_back("REQUEST_METHOD=" + _request->getMethod());
	_envVars.push_back("REQUEST_URI=" + _request->getUri());
	_envVars.push_back("QUERY_STRING=" + _queryString);
	_envVars.push_back("CONTENT_TYPE=" + _request->getHeader("Content-Type"));
	_envVars.push_back("CONTENT_LENGTH=" + _request->getHeader("Content-Length"));
	_envVars.push_back("PATH_INFO=" + _request->getPath());
	_envVars.push_back("REDIRECT_STATUS=200");

	//Client information
	_envVars.push_back("REMOTE_ADDR=127.0.0.1");
	_envVars.push_back("REMOTE_HOST=" + _request->getHeader("Host"));
	_envVars.push_back("HTTP_USER_AGENT=" + _request->getHeader("User-Agent"));
	//_envVars.push_back("HTTP_COOKIE=" + _request->getHeader("Cookie"));
	_envVars.push_back("HTTP_ACCEPT=" + _request->getHeader("Accept"));
	_envVars.push_back("HTTP_ACCEPT_LANGUAGE=" + _request->getHeader("Accept-Language"));

	//Server information
	_envVars.push_back("SERVER_NAME=" + _server->getHost());
	_envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_envVars.push_back("SERVER_PORT=" + intToString(_server->getPort()));
	_envVars.push_back("DOCUMENT_ROOT=" + _server->getRoot());
	_envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");

	//Script information
	_envVars.push_back("SCRIPT_FILENAME=" + _scriptPath);
	_envVars.push_back("SCRIPT_NAME=" + _request->getUri());
}

std::string CGIhandler::resolveScriptPath(const std::string& uri)
{
	return (_server->getCurrentLocation(_request->getPath())->getRoot() + uri);
}

std::string CGIhandler::findInterpreter()
{
	size_t pos = _scriptPath.find_last_of('.');
	if (pos == std::string::npos)
		return (_scriptPath);
	std::string	extention = _scriptPath.substr(pos);
	if (extention == ".php")
		return ("/usr/bin/php-cgi");
	else if (extention == ".py")
		return ("/usr/bin/python3");
	else if (extention == ".pl")
		return ("/usr/bin/perl");
	else if (extention == ".rb")
		return ("/usr/bin/ruby");
	return (_scriptPath);
}

std::string CGIhandler::execute()
{
	// if (access(_interpreter.c_str(), X_OK) != 0) {
	// 	std::cerr << "Interpreter not found or not executable: " << _interpreter << std::endl;
	// 	return "Content-Type: text/html\r\n\r\n<h1>500 Internal Server Error</h1><p>CGI interpreter not found</p>";
	// }

	int inputPipe[2];
	int outputPipe[2];

	if (pipe(inputPipe) || pipe(outputPipe))
		throw std::runtime_error("failed to create pipes");
	int pid = fork();
	if (pid < 0)
		throw std::runtime_error("failed to fork");
	else if (pid == 0)
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

	// If it's a POST request, write the data into the input pipe
	//std::cout << "_request->getMethod() == " << _request->getMethod() << std::endl;
	if (_request->getMethod() == "POST" && !_postData.empty())
	{
		write(inputPipe[1], _postData.c_str(), _postData.length());
		sleep(1);
	}
	close(inputPipe[1]);
	sleep(1);
	// Read the output of the CGI script
	char buffer[4096];
	ssize_t bytesRead;
	std::string output;
	int readCount = 0;

	//std::cout << "Starting to read CGI output..." << std::endl;

	while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer) - 1)) > 0)
	{
		readCount++;
		buffer[bytesRead] = '\0';
		output += buffer;
		//std::cout << "Read iteration " << readCount << ": " << bytesRead << " bytes" << std::endl;
		//std::cout << "Buffer content: [" << buffer << "]" << std::endl;
	}

	//std::cout << "Finished reading. Total length: " << output.length() << std::endl;
	//std::cout << "Final output: [" << output << "]" << std::endl;

	close(outputPipe[0]);

	// Wait for the CGI process to finish
	int status;
	waitpid(pid, &status, 0);

	// Check the status of the end
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		std::cerr << "CGI script exited with status " << WEXITSTATUS(status) << std::endl;
	return output;
}

void CGIhandler::handleRedirection(int inputPipe[2], int outputPipe[2])
{
	close(inputPipe[1]);
	close(outputPipe[0]);
	if (dup2(inputPipe[0], STDIN_FILENO) < 0)
	{
		std::cerr << "dup2 for stdin failed" << std::endl;
		exit(1);
	}
	close(inputPipe[0]);
	if (dup2(outputPipe[1], STDOUT_FILENO) < 0)
	{
		std::cerr << "dup2 for stdout failed" << std::endl;
		exit(1);
	}
	close(outputPipe[1]);
}