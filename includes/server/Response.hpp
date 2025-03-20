#pragma once

#include <string>
#include <map>

class Request; 

class Response
{
private:
    int _statusCode;
    std::string _response;
    std::string _statusMessage;
    std::string _body;
    std::map<std::string, std::string> _headers;
    std::string _httpVersion;
public:
    Response();
    Response(const Request& request);
    ~Response();

    void setStatus(int code);

    std::string getResponse() const { return _response; }
    std::string getStatusMessage() const { return _statusMessage; }
    std::string getBody() const { return _body; }
    std::map<std::string, std::string> getHeaders() const { return _headers; }
};

