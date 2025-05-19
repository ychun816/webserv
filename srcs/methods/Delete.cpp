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
    std::string absPath = request.getAbspath(); //./www/simplesite/upload/_testPost.txt
    std::string uri = request.getUri();            // e.g., /upload/test.txt
    std::string fileName = uri.substr(uri.find_last_of("/") + 1);

    // std::string deletePath = absPath.substr(absPath.find_last_of("/"));

    // std::string fullPath = uploadPath;
    // if (!fullPath.empty())//&& fullPath.back() != '/'
    //     fullPath += "/";
    // fullPath += fileName;

    // DEBUG /////////////////////////////////////////////////
    std::cout << "=== 📌DEBUG DELETE EXECUTE ===" << std::endl;
    std::cout << "Abs PATH : " << absPath << std::endl; //UPLOAD PATH : ./www/simplesite/upload/_testPost.txt
    std::cout << "URI : " << uri << std::endl;
    // std::cout << "DELETE PATH : " << uri << std::endl;
    std::cout << "FILENAME : " << fileName << std::endl;
    std::cout << "FILE TYPE : " << getFileType(absPath) << std::endl;
    std::cout << "=== 📌END | DEBUG DELETE EXECUTE ===" << std::endl;
    /////////////////////////////////////////////////


    if (getFileType(absPath) != TYPE_REGULAR_FILE) 
    {
        response.setStatus(403);
        response.setBody("Error: Not a regular file.");
        return;
    }

    if (std::remove(absPath.c_str()) != 0) 
    {
        // perror("Delete failed");
        response.setStatus(500);
        response.setBody("Error: Failed to delete file.");
    } 
    response.setStatus(200);
    response.setBody("Success: File deleted.");


}


// void Delete::execute(Request& request, Response& response, Server& server)
// {
//     (void)server;
//     std::string uploadFullPath = request.getAbspath();
// 	FileType type = getFileType(uploadFullPath);
//     // std::string uri = request.getUri();
//     // std::string fileName = uri.substr(uri.find_last_of("/") + 1); // get the filename from the uri

//     //DEBUG /////////////////////////////////////////////////
//     std::cout << "=== 📌DEBUG DELETE EXECUTE ===" << std::endl;
//     std::cout << "UPLOAD PATH : " << uploadFullPath << std::endl;
//     std::cout << "FILE TYPE : " << type << std::endl;
//     // std::cout << "REQUEST URI : " << uri << std::endl;
//     // std::cout << "uploadPath.c_str()" << uploadPath.c_str() << std::endl;
//     // std::cout << "fileName : " << fileName << std::endl;
//     std::cout << "=== 📌END | DEBUG DELETE EXECUTE ===" << std::endl;
//     /////////////////////////////////////////////////

//     if (type != TYPE_REGULAR_FILE)
//     {
//         response.setStatus(403);
//         response.setBody("Error: Not a regular file.");
//         return;
//     }

//     if (uploadFullPath.empty())
//     {
//         response.setStatus(400);
//         response.setBody("Error: Bad Request.");
//         return;
//     }


//     //check if the file can be deleted
//     //need to delete frm the full path!!
//     if (std::remove(uploadFullPath.c_str()))
//     {
//         response.setStatus(500);
//         response.setBody("Error: Failed to delete file.");
//     }
//     else
//     {
//         response.setStatus(200);
//         response.setBody("File deleted successfully.");
//     }

// }