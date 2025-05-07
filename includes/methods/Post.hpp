#pragma once

#include "AMethods.hpp"

class Post : public AMethods
{
	public:
		Post();
		// Post(const Post& copy);
		// Post&	operator=(const Post& copy);
		virtual ~Post();

		// virtual void execute(Request& request, Response& response, Server& server);

	private:
};