#pragma once

#include "../server/Request.hpp"
#include "../server/Response.hpp"
#include "../server/Server.hpp"
#include <sys/stat.h>
#include <cstdlib>
#include <vector>
#include <exception>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <dirent.h>

#ifdef _WIN32
	#include <windows.h>
	#include <direct.h>
	#define PATH_SEPARATOR '\\'
	#define getcwd _getcwd
	#define MAX_PATH_LENGTH MAX_PATH
#else
	#include <unistd.h>
	#include <limits.h>
	#include <stdlib.h>
	#include <errno.h>
	#include <string.h>
	#define PATH_SEPARATOR '/'
	#define MAX_PATH_LENGTH PATH_MAX
#endif

enum FileType {
	TYPE_REGULAR_FILE,
	TYPE_DIRECTORY,
	TYPE_OTHER,
	TYPE_NOT_FOUND,
	TYPE_NO_PERMISSION
};

class AMethods
{
	public :
		AMethods();
		AMethods(const AMethods& copy);
		AMethods&	operator=(const AMethods& copy);
		virtual ~AMethods();

		virtual void execute(Request& request, Response& response, Server& server) = 0;

		// Template Method pattern pour le flux commun
		void process(Request& request, Response& response, Server& server)
		{
			if (!checkPath(request))
			{
				std::cerr << "Path error" << std::endl;
				return ;
			}
			try
			{
				execute(request, response, server);
			}
			catch (const std::exception& e)
			{
				handleError(e, response);
			}
		}

		class executeError : public std::exception
		{
			public:
				executeError(const std::string &message) : _message(message) {}
				virtual ~executeError() throw() {}
				virtual const char *what() const throw() { return _message.c_str(); }
			private:
				std::string _message;
		};

	protected :
		bool checkPath(Request& request);
		FileType getFileType(const std::string& path);

	private :
		void handleError(const std::exception& e, Response& response);
};