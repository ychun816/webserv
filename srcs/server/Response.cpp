#include "../../includes/server/Response.hpp"
#include "../../includes/server/Request.hpp"
#include <iostream>
#include <sstream>

Response::Response() :
    _statusCode(0),
    _response(""),
    _statusMessage(""),
    _body(""),
    _headers(),
    _httpVersion("HTTP/1.1"),
    _request(NULL)
{}

Response::Response(const Request& request) :
    _statusCode(0),
    _response(""),
    _statusMessage(""),
    _body(""),
    _headers(),
    _httpVersion("HTTP/1.1"),
    _request(&request)
{}

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

std::string Response::getStatusMessage(size_t code) const{
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 505: return "HTTP Version Not Supported";
        default: return "Unknown Status";
    }
}


std::string Response::formatResponse() const {
    std::stringstream ss;

    // Status line
    ss << _httpVersion << " " << _statusCode << " " << _statusMessage << "\r\n";

    // Existing headers
    std::map<std::string, std::string> finalHeaders = _headers;

    // Si la requête est chunked, on ne met pas Content-Length
    if (_request && _request->isChunked()) {
        finalHeaders.erase("Content-Length");
        finalHeaders["Transfer-Encoding"] = "chunked";
    } else {
        // Add Content-Length if it doesn't exist
        if (finalHeaders.find("Content-Length") == finalHeaders.end()) {
            std::stringstream lenStream;
            lenStream << _body.length();
            finalHeaders["Content-Length"] = lenStream.str();
        }
    }

    // Headers
    for (std::map<std::string, std::string>::const_iterator it = finalHeaders.begin(); 
         it != finalHeaders.end(); ++it) {
        ss << it->first << ": " << it->second << "\r\n";
    }

    ss << "\r\n";

    // Si chunked, formater le body en chunks
    if (_request && _request->isChunked()) {
        std::stringstream chunkedBody;
        size_t pos = 0;
        const size_t chunkSize = 8192; // 8KB chunks

        while (pos < _body.length()) {
            size_t currentChunkSize = std::min(chunkSize, _body.length() - pos);
            chunkedBody << std::hex << currentChunkSize << "\r\n";
            chunkedBody << _body.substr(pos, currentChunkSize) << "\r\n";
            pos += currentChunkSize;
        }
        // Fin du transfert chunked
        chunkedBody << "0\r\n\r\n";
        ss << chunkedBody.str();
    } else {
        ss << _body;
    }

    return ss.str();
}