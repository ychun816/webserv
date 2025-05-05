#include "../../includes/methods/CGIhandler.hpp"

CGIHandler::CGIHandler(const Request& request, const Server& config)
	: _request(request), _config(config)
{
	_scriptPath = resolveScriptPath(request.getUri());
	_interpreter = findInterpreter();
	setupEnvironment();
}

CGIHandler::~CGIHandler() {}

std::string CGIHandler::execute()
{
	int inputPipe[2];
	int outputPipe[2];

	// Créer les pipes pour la communication
	if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0) {
		throw std::runtime_error("Failed to create pipes");
	}

	pid_t pid = fork();
	if (pid < 0) {
		throw std::runtime_error("Fork failed");
	}
	if (pid == 0)
	{
		close(inputPipe[1]);
		close(outputPipe[0]);

		handleRedirection(inputPipe, outputPipe);
		char* args[] = {(char*)_interpreter.c_str(), (char*)_scriptPath.c_str(), NULL};
		std::vector<char*> env;
		for (size_t i = 0; i < _envVars.size(); i++)
		{
			env.push_back((char*)_envVars[i].c_str());
		}
		env.push_back(NULL);
		execve(_interpreter.c_str(), args, &env[0]);
		std::cerr << "Execve failed: " << strerror(errno) << std::endl;
		exit(1);
	}
	close(inputPipe[0]);
	close(outputPipe[1]);
	if (_request.getMethod() == "POST" && !_request.getBody().empty()) {
		write(inputPipe[1], _request.getBody().c_str(), _request.getBody().length());
	}
	close(inputPipe[1]);
	std::string output;
	char buffer[1024];
	ssize_t bytesRead;
	while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
		output.append(buffer, bytesRead);
	}
	close(outputPipe[0]);
	int status;
	waitpid(pid, &status, 0);

	return output;
}

void CGIHandler::setupEnvironment()
{
	_envVars.clear();

	_envVars.push_back("REQUEST_METHOD=" + _request.getMethod());
	_envVars.push_back("QUERY_STRING=" + _request.getQueryString());
	_envVars.push_back("CONTENT_TYPE=" + _request.getHeader("Content-Type"));
	_envVars.push_back("CONTENT_LENGTH=" + toString(_request.getBody().length()));
	_envVars.push_back("SERVER_NAME=" + _config.getHost());
	_envVars.push_back("SERVER_PORT=" + toString(_config.getPort()));
	_envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	_envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");

	// Variables supplémentaires pour les en-têtes HTTP
	const std::map<std::string, std::string>& headers = _request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it)
	{
		std::string headerVar = "HTTP_" + it->first;
		// Convertir le nom de l'en-tête en format CGI
		for (size_t i = 0; i < headerVar.length(); i++) {
			if (headerVar[i] == '-') headerVar[i] = '_';
			headerVar[i] = toupper(headerVar[i]);
		}
		_envVars.push_back(headerVar + "=" + it->second);
	}
}

std::string CGIHandler::findInterpreter()
{
	size_t lastDot = _scriptPath.find_last_of('.');
	if (lastDot != std::string::npos)
	{
		std::string ext = _scriptPath.substr(lastDot);
		if (ext == ".php")
			return "/usr/bin/php";
		else if (ext == ".py")
			return "/usr/bin/python";
		else if (ext == ".pl")
			return "/usr/bin/perl";
		else if (ext == ".rb")
			return "/usr/bin/ruby";
	}
	return _scriptPath;
}

void CGIHandler::handleRedirection(int inputPipe[2], int outputPipe[2])
{
	close(inputPipe[1]);
	if (dup2(inputPipe[0], STDIN_FILENO) < 0)
	{
		perror("dup2 failed on stdin");
		exit(1);
	}
	close(inputPipe[0]);
	close(outputPipe[0]);
	if (dup2(outputPipe[1], STDOUT_FILENO) < 0)
	{
		perror("dup2 failed on stdout");
		exit(1);
	}
	close(outputPipe[1]);
}

std::string CGIHandler::resolveScriptPath(const std::string& uri)
{
	return _config.getRoot() + uri;
}

std::string CGIHandler::toString(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}