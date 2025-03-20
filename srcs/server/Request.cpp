#include "../../includes/server/Request.hpp"
#include "../../includes/server/Response.hpp"
#include <sstream>
#include <iostream>

Request::Request(std::string request) : _request(request)
{
    parseRequest();

}


Request::~Request()
{

}

void Request::handleResponse()
{
    Response response;
}

void Request::parseRequest()
{
    std::stringstream ss(_request);
    std::string line;
    
    // Parse la première ligne (méthode, URL, version)
    if (std::getline(ss, line) && !line.empty()) {
        parseRequestLine(line);
        std::cout << "method: " << _method << std::endl;
        std::cout << "uri: " << _uri << std::endl;
        std::cout << "http version: " << _httpVersion << std::endl;
    }
    
    // Parse les en-têtes
    while (std::getline(ss, line) && !line.empty() && line != "\r") {
        if (line.back() == '\r') line.pop_back(); // Enlever le CR si présent
        parseHeader(line);
    }

    // Le reste est le corps de la requête
    std::string body;
    while (std::getline(ss, line)) {
        body += line + "\n";
    }
    _body = body;

    handleResponse();
}

void Request::parseRequestLine(const std::string& line)
{
    std::istringstream iss(line);
    iss >> _method >> _uri >> _httpVersion;
    // Supprimer le "\r" si présent dans la version HTTP
    if (!_httpVersion.empty() && _httpVersion.back() == '\r')
        _httpVersion.pop_back();
}

void Request::parseHeader(const std::string& line)
{
    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        // Supprimer les espaces en début de valeur
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);
        _headers[key] = value;
    }
}