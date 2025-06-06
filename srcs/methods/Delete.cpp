#include "../../includes/methods/Delete.hpp"

Delete::Delete() : AMethods::AMethods() {}
Delete::~Delete() {}

/**
 * 1 get path
 * 2 check file type -> if file not accessible => error(403) (Client Err: Forbidden — Access denied)
 * 3 if file ok -> .remove() -> if fail => error(500) (Server Err: Internal Server Error)
 * @note std::move(*i) => takes character -> need .c_str()
 */
void Delete::execute(Request& request, Response& response, Server& server)
{
    (void)server;
    std::string absPath = request.getAbspath();    
    std::string uri = request.getUri();            
    std::string fileName = uri.substr(uri.find_last_of("/") + 1);

    if (getFileType(absPath) != TYPE_REGULAR_FILE) 
    {
        std::cout << "Path not safe: " << absPath << std::endl;
        response.setStatus(403);
        return;
    }

    if (std::remove(absPath.c_str()) != 0) 
    {
        std::cout << "Failed to delete file: " << absPath << std::endl;
        response.setStatus(500);
        return;

    } 
    response.setStatus(200);    
}


/* DEBUGGER

    // DEBUG /////////////////////////////////////////////////
    std::cout << "=== 📌DEBUG DELETE EXECUTE ===" << std::endl;
    std::cout << "📌Abs PATH : " << absPath << std::endl; //UPLOAD PATH : ./www/simplesite/upload/_testPost.txt
    std::cout << "📌URI : " << uri << std::endl;
    // std::cout << "DELETE PATH : " << uri << std::endl;
    std::cout << "📌FILENAME : " << fileName << std::endl;
    std::cout << "📌FILE TYPE : " << getFileType(absPath) << std::endl;
    std::cout << "=== 📌END | DEBUG DELETE EXECUTE ===" << std::endl;
    /////////////////////////////////////////////////

*/