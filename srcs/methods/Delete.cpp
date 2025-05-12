#include "../../includes/methods/Delete.hpp"

Delete::Delete() : AMethods::AMethods() {}
// Delete::Delete(const Delete& copy) : AMethods::AMethods(copy) {}
// Delete&	Delete::operator=(const Delete& copy) {return *this;}
Delete::~Delete() {}

/**
 * 1 get path
 * 2 check file type -> if file not accessible => error(403) (Client Err: Forbidden — Access denied) 
 * 3 if file ok -> .remove() 
 * -> if fail => error(500) (Server Err: Internal Server Error)
 * -> success => 200
 * 
 * @note std::move(*i) => takes character -> need .c_str()
 * 
 */
void Delete::execute(Request& request, Response& response, Server& server)
{
    (void)server;
    std::string path = request.getPath();
	FileType type = getFileType(path);
    
    if (type != TYPE_REGULAR_FILE)
    {
        response.setStatus(403);
        //return;  //maybe no need?
    }

    if (checkPath(request) == false)
        return;
    
    if (!std::remove(path.c_str())) 
    {
        response.setStatus(500);
        // return ; //maybe no need return?
    }
    else
        response.setStatus(200);

}

// bool AMethods::checkPath(Request& request)
// {
// 	if (!pathExist(request))
// 		return (false);
// 	if (!isPathSafe(get_absolute_path(request)))
// 		return (false);
// 	return (true);
// }