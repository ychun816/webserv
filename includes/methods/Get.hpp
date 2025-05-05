#pragma once

#include "AMethods.hpp"

class Get : public AMethods
{
	public:
		Get();
		Get(const Get& copy);
		Get&	operator=(const Get& copy);
		virtual ~Get();

		virtual void execute(Request& request, Response& response);

	private:
};