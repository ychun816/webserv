#include "../../includes/methods/Delete.hpp"

Delete::Delete() : AMethods::AMethods() {}

Delete::Delete(const Delete& copy) : AMethods::AMethods(copy) {}

Delete&	Delete::operator=(const Delete& copy) {return *this;}

Delete::~Delete() {}

void Delete::execute(Request& request, Response& response)
{
}