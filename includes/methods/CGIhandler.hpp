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
#include <filesystem>
#include <iostream>

class CGIhandler
{
	public:
		CGIhandler(Request* request, Server* config);
		CGIhandler(const CGIhandler& copy);
		CGIhandler&	operator=(const CGIhandler& copy);
		~CGIhandler();

		std::string execute();
		std::string	getScriptPath() {return _scriptPath;}

	private:
		std::string _scriptPath;
		std::string _interpreter;
		std::vector<std::string> _envVars;
		std::string _queryString;
		std::string _postData;
		Request* _request;
		Server* _server;


		void setupEnvironment();
		std::string findInterpreter();
		void handleRedirection(int inputPipe[2], int outputPipe[2]);
		std::string resolveScriptPath(const std::string& uri);
};