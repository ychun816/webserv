#include "../../includes/server/Response.hpp"
#include "../../includes/server/Request.hpp"

Response::Response() : 
    _statusCode(0),
    _response(""),
    _statusMessage(""),
    _body(""),
    _headers(),
    _httpVersion("HTTP/1.1")
{}

Response::Response(const Request& request) : 
    _statusCode(0),
    _response(""),
    _statusMessage(""),
    _body(""),
    _headers(),
    _httpVersion("HTTP/1.1")
{
    (void)request;  // Pour éviter l'avertissement de paramètre non utilisé
}

Response::~Response()
{
}

void Response::setStatus(int code)
{
    _statusCode = code;
    
    // Mettre à jour le message de statut en fonction du code
    switch (code) {
        case 200: _statusMessage = "OK"; break;
        case 201: _statusMessage = "Created"; break;
        case 204: _statusMessage = "No Content"; break;
        case 301: _statusMessage = "Moved Permanently"; break;
        case 302: _statusMessage = "Found"; break;
        case 400: _statusMessage = "Bad Request"; break;
        case 401: _statusMessage = "Unauthorized"; break;
        case 403: _statusMessage = "Forbidden"; break;
        case 404: _statusMessage = "Not Found"; break;
        case 405: _statusMessage = "Method Not Allowed"; break;
        case 413: _statusMessage = "Payload Too Large"; break;
        case 500: _statusMessage = "Internal Server Error"; break;
        case 501: _statusMessage = "Not Implemented"; break;
        case 505: _statusMessage = "HTTP Version Not Supported"; break;
        default: _statusMessage = "Unknown Status"; break;
    }
}