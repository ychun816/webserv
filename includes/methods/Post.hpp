#pragma once

#include "AMethods.hpp"

class Post : public AMethods
{
	public:
		Post();
		virtual ~Post();

		virtual void execute(Request& request, Response& response, Server& server);

	private:
		std::string extractFileContent(std::string& rawBody) const;

};