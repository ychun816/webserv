#include "../../includes/methods/Post.hpp"
#include "../../includes/server/Request.hpp"
#include "../../includes/utils/Utils.hpp"

Post::Post() : AMethods::AMethods() {}
Post::~Post() {}

std::string Post::extractFileContent(std::string& rawBody) const
{
    // Start after headers
    size_t contentStart = rawBody.find("\r\n\r\n");
    if (contentStart == std::string::npos)
        return "";
    contentStart += 4; // Skip "\r\n\r\n"

    // Find start of the final boundary
    size_t contentEnd = rawBody.rfind("\r\n--");
    if (contentEnd == std::string::npos || contentEnd < contentStart)
        return "";

    std::string content = rawBody.substr(contentStart, contentEnd - contentStart);

    // Trim trailing \r\n
    content.erase(content.find_last_not_of("\r\n") + 1);

    return content;
}

void Post::execute(Request& request, Response& response, Server& server)
{
    if (!request.isBodySizeValid()) 
    {
        if (!request.errorPageExist(413)) 
        {
            response.setStatus(413);
            request.buildErrorPageHtml(response.getStatus(), response);
        }
        else 
            request.openErrorPage(413, response);
        return;
    }
    
    try {
        (void)server;
        std::string uploadPath;
        std::string filename;
        std::string body;

        // Change the path to the upload_path in the configfile
	    Location* _currentLocation = server.getCurrentLocation(request.getPath());
        uploadPath = "." + server.getRoot() + _currentLocation->getUploadPath();
        std::cout << "Upload path ver" << uploadPath << std::endl;
        if (uploadPath.empty()) 
        {
            response.setStatus(400);
            return;
        }

        filename = request.getFilename();
        if (filename.empty()) 
        {
            response.setStatus(400);
            return;
        }
        
        body = request.getBody();
        if (!body.empty())
            body = extractFileContent(body);

        std::string uri = request.getUri(); //test with change /uplaoad to /user_upload
        // std::string newSavePath = "/user_upload";

        std::string savePath = uploadPath + PATH_SEPARATOR + filename;
        std::ofstream output(savePath.c_str());

        if (!output.is_open()) {
            response.setStatus(500);
            return;
        }
        output << body;
        output.close();

        response.setStatus(201);
        response.setBody("Success: File uploaded.\n");
        request.fillResponse(response, 201, "<html><body><h1>Success: File uploaded.</h1></body></html>");
    }
    catch (const std::exception& e) 
    {
        // Safely handle any exceptions that might occur
        std::cerr << "Exception in POST handler: " << e.what() << std::endl;
        response.setStatus(500);
    }
}


/* ERROR PAGE CODES
 * 
 * HTTP status codes are grouped into five classes:
 * 1xx: Informational
 * 2xx: Success
 * 3xx: Redirection
 * 4xx: Client Error
 * 5xx: Server Error
 * 
| Code    | Type        | Meaning (EN)                         | 中文說明 |
| ------- | ----------- | ------------------------------------ | ------- |
| **200** | Success     | OK — The request has succeeded       | 請求成功，伺服器已回應資料 |
| **201** | Success     | Created — New resource created       | 資源已成功建立 |
| **204** | Success     | No Content — No body in response     | 無內容，通常用於成功但無需回傳資料 |
| **301** | Redirect    | Moved Permanently                    | 永久轉址    |
| **302** | Redirect    | Found (Temporary Redirect)           | 暫時轉址 |
| **304** | Redirect    | Not Modified — Use cached version    | 資料未變更，使用快取版本 |
| **400** | Client Err. | Bad Request — Syntax error           | 請求格式錯誤 |
| **401** | Client Err. | Unauthorized — Login required        | 未授權，需要驗證 |
| **403** | Client Err. | Forbidden — Access denied            | 禁止存取，權限不足 |
| **404** | Client Err. | Not Found — Resource doesn't exist   | 找不到資源 |
| **405** | Client Err. | Method Not Allowed                   | 不支援的 HTTP 方法 |
| **408** | Client Err. | Request Timeout                      | 請求逾時 |
| **429** | Client Err. | Too Many Requests                    | 請求太頻繁，被限制 |
| **500** | Server Err. | Internal Server Error                | 伺服器內部錯誤 |
| **502** | Server Err. | Bad Gateway — Invalid upstream resp. | 網關錯誤 |
| **503** | Server Err. | Service Unavailable                  | 服務暫停 |
| **504** | Server Err. | Gateway Timeout                      | 網關逾時 |

*/

/* DEBUGGER 
   
        //DEBUG  /////////////////////////////////////////////////
        std::cout << "=== ♦️DEBUG POST EXECUTE ===" << std::endl;
        // std::cout << "UPLOAD DIRECTORY : " << request.getUploadDirectory(uri) << std::endl; //SAVED HERE FOR FUTRE USE
        std::cout << "♦️UPLOAD PATH : " << uploadPath << std::endl;
        std::cout << "♦️FILENAME : " << filename << std::endl;
        std::cout << "♦️SAVE PATH : " << savePath << std::endl;//////////////vePath << std::endl;
	    std::cout << "♦️URI: " << uri << std::endl;
        // std::cout << "♦️BODY : " << body << std::endl;
        std::cout << "=== ♦️END | DEBUG POST EXECUTE ===" << std::endl;
        /////////////////////////////////////////////////


//POST EXECUTE
        //DEBUG 
        std::cout << "=== ♦️DEBUG POST EXECUTE ===" << std::endl;
        // std::cout << "UPLOAD DIRECTORY : " << request.getUploadDirectory(uri) << std::endl; //SAVED HERE FOR FUTRE USE
        std::cout << "♦️UPLOAD PATH : " << uploadPath << std::endl;
        std::cout << "♦️FILENAME : " << filename << std::endl;
        std::cout << "♦️SAVE PATH : " << sa//////////////vePath << std::endl;
	    std::cout << "♦️URI: " << uri << std::endl;
        std::cout << "♦️BODY : " << body << std::endl;
        std::cout << "=== ♦️END | DEBUG POST EXECUTE ===" << std::endl;
        /////////////////////////////////////////////////

*/