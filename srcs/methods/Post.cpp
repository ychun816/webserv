#include "../../includes/methods/Post.hpp"
#include "../../includes/server/Request.hpp"
#include "../../includes/utils/Utils.hpp"

/* TO DO NOTE:
1. upload file can save to another temp directory OR download to the same directory
2. have to test download the uploaded file
*/




Post::Post() : AMethods::AMethods() {}
// Post::Post(const Post& copy) : AMethods::AMethods(copy) {}
// Post&	Post::operator=(const Post& copy) {return *this;}
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

    //CHECK DEBUG
    // std::cout << "=== 🍋DEBUG | EXTRACT FILE CONTENT ===" << std::endl;
    // std::cout << "HEADER DELIMITER : " << headerDelimiter << std::endl;
    // std::cout << "CONTENT START : " << contentStart << std::endl;
    // std::cout << "CONTENT END : " << contentEnd << std::endl;
    // std::cout << "CONTENT : " << content << std::endl;
    // std::cout << "=== 🍋END | DEBUG EXTRACT FILE CONTENT ===" << std::endl;

    return content;
}


void Post::execute(Request& request, Response& response, Server& server)
{
    if (!request.isBodySizeValid()) {
        response.setStatus(413);
        request.buildErrorPageHtml(response.getStatus(), response);
        return;
    }
    
    try {
        std::string uploadPath;
        std::string filename;
        std::string body;

        (void)server;
        // Change the path to the upload_path in the configfile
        uploadPath = request.getAbspath();
        if (uploadPath.empty()) {
            response.setStatus(400);
            request.buildErrorPageHtml(response.getStatus(), response);
            return;
        }

        filename = request.getFilename();
        if (filename.empty()) {
            response.setStatus(400);
            request.buildErrorPageHtml(response.getStatus(), response);
            return;
        }
        
        body = request.getBody();
        if (!body.empty()) {
            body = extractFileContent(body);
        }

        //test with change /uplaoad to /user_upload
        std::string uri = request.getUri();
        // std::string newSavePath = "/user_upload";


        std::string savePath = uploadPath + PATH_SEPARATOR + filename;
        std::ofstream output(savePath.c_str());

        //DEBUG /////////////////////////////////////////////////
        std::cout << "=== ♦️DEBUG POST EXECUTE ===" << std::endl;
        std::cout << "♦️UPLOAD PATH : " << uploadPath << std::endl;
        std::cout << "♦️FILENAME : " << filename << std::endl;
        std::cout << "♦️SAVE PATH : " << savePath << std::endl;
	    std::cout << "♦️URI: " << uri << std::endl;
        std::cout << "♦️BODY : " << body << std::endl;
        std::cout << "=== ♦️END | DEBUG POST EXECUTE ===" << std::endl;
        /////////////////////////////////////////////////

        if (!output.is_open()) {
            response.setStatus(500);
            request.buildErrorPageHtml(response.getStatus(), response);
            return;
        }

        output << body;
        output.close();

        response.setStatus(201);
        response.setBody("Success: File uploaded.\n");
        request.fillResponse(response, 201, "<html><body><h1>Success: File uploaded.</h1></body></html>");
    }
    catch (const std::exception& e) {
        // Safely handle any exceptions that might occur
        std::cerr << "Exception in POST handler: " << e.what() << std::endl;
        response.setStatus(500);
        request.buildErrorPageHtml(response.getStatus(), response);
    }
}



// std::string Request::getFilename() const
// {
//     std::string filename;
//     size_t pos = _body.find("filename=\""); //std::string::size_type pos
// 	// std::cout << ">>>FILENAME POS : " << pos << std::endl; //DEBUG

//     if (pos != std::string::npos)
//     {
//         //found
//         pos += 10; //skip filename="
//         size_t endPos = _body.find("\"", pos);//start find frm pos
		
// 		// std::cout << ">>>FILENAME ENDPOS : " << pos << std::endl; //DEBUG

//         if (endPos != std::string::npos)
// 		    filename = _body.substr(pos, endPos - pos);
// 		// std::cout << ">>>FILENAME  : " << filename << std::endl; //DEBUG
//     }
//     return filename;
// }





/*

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


    error_page=404 error/404.html V
    error_page=500 error/500.html V
    error_page=403 error/403.html V
    error_page=504 error/504.html V
    error_page=405 error/405.html V
    error_page=400 error/400.html V
    error_page=408 error/408.html V
    error_page=413 error/413.html
    error_page=501 error/501.html

*/