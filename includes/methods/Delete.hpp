#pragma once

#include "AMethods.hpp"

class Delete : public AMethods
{
	public:
		Delete();
		Delete(const Delete& copy);
		Delete&	operator=(const Delete& copy);
		virtual ~Delete();

		virtual void execute(Request& request, Response& response) = 0;

	private:
};