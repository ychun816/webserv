#include "../../includes/methods/Post.hpp"

Post::Post() : AMethods::AMethods() {}
// Post::Post(const Post& copy) : AMethods::AMethods(copy) {}
// Post&	Post::operator=(const Post& copy) {return *this;}
Post::~Post() {}

void Post::execute(Request& request, Response& response, Server& server)
{
    handleUpload(request, response, server);
}

/**
 * 1 get uploadpath / filename / body 
 * 2 Create a file stream for output, using full path
 * 3 check if can create file
 * - if not -> error msg 
 * - if yes -> write body to file
 * 4 close() + set status success 
 * 5 set status and success message 
 * 
 * @note std::ofstream out(uploadPath + PATH_SEPARATOR + filename.c_str()) : Create a file stream for output, using full path
 */

void Post::handleUpload(Request& request, Response& response, Server& server)
{
    std::string uploadPath;
    std::string filename;
    std::string body;

    uploadPath = server.getUpload();
    filename = request.getFilename();
    body = request.getBody();

    // Create a file stream for output, using full path
    std::string full_path = uploadPath + PATH_SEPARATOR + filename;
    std::ofstream output(full_path.c_str());

    //check if the file can be created
    if (!output.is_open())
    {
        //Internal server error 500
        //set error msg
        response.setStatus(500);
        response.setBody("Error : Failed saving file.\n");
        return; 
    }
    output << body;
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