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
    std::string absPath = request.getAbspath();    //./www/simplesite/upload/_testPost.txt
    std::string uri = request.getUri();            // e.g., /upload/test.txt
    std::string fileName = uri.substr(uri.find_last_of("/") + 1);

    if (getFileType(absPath) != TYPE_REGULAR_FILE) 
    {
        if (!request.getHavePriority())
        {
            std::cout << "Path not safe: " << absPath << std::endl;
            if (!request.errorPageExist(403)) {
                response.setStatus(403);
                request.buildErrorPageHtml(403, response);
            } else {
                request.openErrorPage(403, response);
            }
        }
        else
        {
            std::cout << " I dont have priority, so I will not handle this error" << std::endl;
        }
        return;
    }

    if (std::remove(absPath.c_str()) != 0) 
    {
        if (!request.getHavePriority())
        {
            std::cout << "Failed to delete file: " << absPath << std::endl;
            if (!request.errorPageExist(500)) {
                response.setStatus(500);
                request.buildErrorPageHtml(500, response);
            } else {
                request.openErrorPage(500, response);
            }
        }
        else
        {
            std::cout << " I dont have priority, so I will not handle this error" << std::endl;
        }
        return;

    } 
    response.setStatus(200);
    response.setBody("Success: File deleted.");
    request.fillResponse(response, 200, "<html><body><h1>Success: File deleted.</h1></body></html>");
    
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