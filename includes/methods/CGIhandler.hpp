#pragma once

#include "Request.hpp"
#include "Server.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <cctype>
#include <cstring>

class CGIHandler
{
	public:
		CGIHandler(const Request& request, const Server& config);
		~CGIHandler();

		std::string execute();

	private:
		std::string _scriptPath;
		std::string _interpreter;
		std::vector<std::string> _envVars;
		std::string _queryString;
		std::string _postData;
		const Request& _request;
		const Server& _config;

		void setupEnvironment();
		std::string findInterpreter();
		void handleRedirection(int inputPipe[2], int outputPipe[2]);

		std::string resolveScriptPath(const std::string& uri);
		std::string toString(int value);
};