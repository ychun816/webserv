#pragma once

#include "AMethods.hpp"

class Delete : public AMethods
{
	public:
		Delete();
		virtual ~Delete();

		virtual void execute(Request& request, Response& response, Server& server);

};