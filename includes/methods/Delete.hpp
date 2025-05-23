#pragma once

#include "AMethods.hpp"

/*
- remove a file /OR/ deny the request if not allowed
- return a proper HTTP response
*/

class Delete : public AMethods
{
	public:
		Delete();
		// Delete(const Delete& copy);
		// Delete&	operator=(const Delete& copy);
		virtual ~Delete();

		virtual void execute(Request& request, Response& response, Server& server);

};