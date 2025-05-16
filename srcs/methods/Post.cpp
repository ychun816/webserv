#include "../../includes/methods/Post.hpp"
#include "../../includes/server/Request.hpp"

Post::Post() : AMethods::AMethods() {}
// Post::Post(const Post& copy) : AMethods::AMethods(copy) {}
// Post&	Post::operator=(const Post& copy) {return *this;}
Post::~Post() {}


//extract file content
std::string Post::extractFileContent(std::string& rawBody) const 
{
    std::string delimiter = "\r\n\r\n"; // End of headers
    size_t contentStart = rawBody.find(delimiter);
    if (contentStart == std::string::npos) return "";

    contentStart += delimiter.length();

    size_t contentEnd = rawBody.rfind("--"); // Last boundary
    if (contentEnd == std::string::npos || contentEnd <= contentStart) return "";

    return rawBody.substr(contentStart, contentEnd - contentStart);
}

// std::string Post::extractFileContent(std::string& rawBody) const
// {
//     std::string delimiter = "\r\n\r\n"; // Delimiter for the file content
//     size_t delimiterLen = delimiter.length();
    
//     size_t headerEnd = rawBody.find(delimiter);
//     if (headerEnd == std::string::npos)
//         return "";

//     size_t contentStart = headerEnd + delimiterLen;
//     contentStart += 
//     size_t contentEnd = rawBody.rfind("--");// Strip last boundary
//     if (contentEnd == std::string::npos || contentEnd < contentStart)
//         return "";

//     std::string content = rawBody.substr(contentStart, contentEnd - contentStart);

//     //CHECK DEBUG
//     std::cout << "=== 🍋DEBUG | EXTRACT FILE CONTENT ===" << std::endl;
//     std::cout << "DELIMITER : " << delimiter << std::endl;
//     std::cout << "DELIMITER LEN : " << delimiterLen << std::endl;
//     std::cout << "CONTENT START : " << contentStart << std::endl;
//     std::cout << "CONTENT END : " << contentEnd << std::endl;
//     std::cout << "CONTENT : " << content << std::endl;
//     std::cout << "=== 🍋END | DEBUG EXTRACT FILE CONTENT ===" << std::endl;

//     return content;
// }

void Post::execute(Request& request, Response& response, Server& server)
{
    std::string uploadPath;
    std::string filename;
    std::string rawBody;
    std::string body;

    (void)server;
    uploadPath = request.getAbspath(); //+ server.getUploadPath();
    filename = request.getFilename();
    rawBody = request.getBody();
    body = extractFileContent(rawBody);

    // Create a file stream for output, using full path
    std::string full_path = uploadPath + PATH_SEPARATOR + filename;
    std::ofstream output(full_path.c_str());

    //DEBUG /////////////////////////////////////////////////
    std::cout << "=== 📍DEBUG POST EXECUTE ===" << std::endl;
    std::cout << "UPLOAD PATH : " << uploadPath << std::endl;
    std::cout << "FILENAME : " << filename << std::endl;
    std::cout << "FULL PATH : " << full_path << std::endl; //shold be ok
    std::cout << "RAW BODY : " << rawBody << std::endl;
    std::cout << "BODY : " << body << std::endl;
    std::cout << "=== 📍END | DEBUG POST EXECUTE ===" << std::endl;
    /////////////////////////////////////////////////
    
    //check if the file can be created
    if (!output.is_open())
    {
        response.setStatus(500);
        response.setBody("Error : Failed saving file.\n");
        return;
    }
    output << body;
    std::cout << "📍📍OUTPUT : " << output << std::endl;
    output.close();

    response.setStatus(201);
    response.setBody("Success : file uploaded.\n");

}




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
| **404** | Client Err. | Not Found — Resource doesn’t exist   | 找不到資源 |
| **405** | Client Err. | Method Not Allowed                   | 不支援的 HTTP 方法 |
| **408** | Client Err. | Request Timeout                      | 請求逾時 |
| **429** | Client Err. | Too Many Requests                    | 請求太頻繁，被限制 |
| **500** | Server Err. | Internal Server Error                | 伺服器內部錯誤 |
| **502** | Server Err. | Bad Gateway — Invalid upstream resp. | 網關錯誤 |
| **503** | Server Err. | Service Unavailable                  | 服務暫停 |
| **504** | Server Err. | Gateway Timeout                      | 網關逾時 |

*/