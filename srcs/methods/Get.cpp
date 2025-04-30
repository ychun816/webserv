#include "../../includes/methods/Get.hpp"

Get::Get() : AMethods::AMethods() {}

Get::Get(const Get& copy) : AMethods::AMethods(copy) {}

Get&	Get::operator=(const Get& copy) {return *this;}

Get::~Get() {}

void Get::execute(Request& request, Response& response)
{
	std::string uri = request.getUri();
	if (!checkPath(uri))
		return ;
	FileType file_type = getFileType(uri);
	switch (file_type)
	{
		case (TYPE_REGULAR_FILE) : break;
		case (TYPE_DIRECTORY) : break;
		case (TYPE_OTHER) : break;
		case (TYPE_NOT_FOUND) : break;
		case (TYPE_NO_PERMISSION) : break;
		default: break;
	}
}
