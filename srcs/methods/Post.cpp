#include "../../includes/methods/Post.hpp"

Post::Post() : AMethods::AMethods() {}

Post::Post(const Post& copy) : AMethods::AMethods(copy) {}

Post&	Post::operator=(const Post& copy) {return *this;}

Post::~Post() {}

void Post::execute(Request& request, Response& response)
{
}