#include "../../includes/server/Request.hpp"
#include "../../includes/server/Response.hpp"
#include "../../includes/server/Server.hpp"
#include "../../includes/utils/Utils.hpp"
#include "../../includes/parsing/Config.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>  // Pour std::find
#include <cstdlib>  // Pour atoi

Request::Request(std::string request, Server& server) :
	_server(server),
	_response(),
	_request(request),
	_method(""),
	_uri(""),
	_httpVersion(""),
	//_queryString(""),
	_path(""),
	_headers(),
	_body(""),
	_currentLocation(NULL),
	_isChunked(false)
{
	parseRequest();
	setPathQueryString();
	parseQuery();
	_currentLocation = _server.getCurrentLocation(_path);
            std::vector<std::string> methods = _currentLocation->getMethods();
            std::cout << "Méthodes autorisées pour cette location : ";
            for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); ++it) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
}

Request::~Request()
{
}

void Request::handleResponse()
{
	if (!isContentLengthValid()) {
		Response response(*this);
		std::cout << "👻 Content-Length is not valid" << std::endl;
		response.setStatus(413); // Payload Too Large
		return;
	}

	if (!isMethodAllowed()) {
		Response response(*this);
		response.setStatus(405); // Method Not Allowed
		return;
	}

	Response response(*this);
	
	// Obtenir le port du serveur actuel
	int port = _server.getPort();
	Config* config = Config::getInstance();
	if (config)
	{
		Server* appropriateServer = config->findServerByLocation(_path, port);
		if (appropriateServer)
			_server = *appropriateServer;
	}
	_server.executeMethods(*this, response);
	response.setResponse(response.formatResponse());
	
	std::cout << BLUE << "Sending response: [" << response.getResponse().substr(0, response.getResponse().length()) << "...]" << RESET << std::endl;
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
		if (!line.empty() && line[line.length() - 1] == '\r')
			line = line.substr(0, line.length() - 1); // Remove CR if present
		parseHeader(line);
	}

	_isChunked = (getHeader("Transfer-Encoding") == "chunked");
	
	if (_isChunked) {
		parseChunkedBody();
	} else {
		std::string body;
		while (std::getline(ss, line)) {
			body += line + "\n";
		}
		_body = body;
	}
	std::cout << "Received request: [" << this->getBody().substr(0, this->getBody().length()) << "...]" << std::endl;
	//handleResponse();

}

void Request::parseRequestLine(const std::string& line)
{
	std::istringstream iss(line);
	iss >> _method >> _uri >> _httpVersion;
	// Remove "\r" if present in the HTTP version
	if (!_httpVersion.empty() && _httpVersion[_httpVersion.length() - 1] == '\r')
		_httpVersion = _httpVersion.substr(0, _httpVersion.length() - 1);
}

void Request::parseHeader(const std::string& line)
{
	size_t colonPos = line.find(':');
	if (colonPos != std::string::npos) {
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		// Remove spaces at the beginning of the value
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);
		_headers[key] = value;
	}
}

std::string Request::getHeader(const std::string& name) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(name);
	if (it != _headers.end())
		return it->second;
	return "";
}

void	Request::parseQuery()
{
	size_t start = 0;
	size_t ampPos;

	while (start < _queryString.length())
	{
		ampPos = _queryString.find('&', start);
		if (ampPos == std::string::npos)
			ampPos = _queryString.length();
		std::string pair = _queryString.substr(start, ampPos - start);
		size_t equalPos = pair.find('=');
		if (equalPos != std::string::npos)
		{
			std::string key = pair.substr(0, equalPos);
			std::string value = pair.substr(equalPos + 1);
			_queryParams[key] = value;
		}
		start = ampPos + 1;
	}
}

void	Request::setPathQueryString()
{
	size_t qPos = _uri.find('?');
	if (qPos != std::string::npos)
	{
		_path = _uri.substr(0, qPos);
		_queryString = _uri.substr(qPos + 1);
		return ;
	}
	_path = _uri;
	_queryString = "";
}

bool Request::validateQueryParams()
{
	for (std::map<std::string, std::string>::const_iterator it = _queryParams.begin(); it != _queryParams.end(); ++it)
	{
		const std::string& key = it->first;
		const std::string& value = it->second;

		if (key == "id" && !isValidInt(value))
			return false;
		else if (key == "page" && !isValidInt(value))
			return false;
		else if (key == "limit" && !isValidInt(value))
			return false;
		else if (key == "email" && !isValidEmail(value))
			return false;
		else if (key == "active" && !isValidBool(value))
			return false;
	}
	return true;
}

bool Request::isValidInt(const std::string& value)
{
	for (size_t i = 0; i < value.length(); i++)
	{
		if (!isdigit(value[i]))
			return false;
	}
	return !value.empty();
}

bool Request::isValidBool(const std::string& value)
{
	return (value == "true" || value == "false" ||
			value == "1" || value == "0" ||
			value == "yes" || value == "no");
}

bool Request::isValidEmail(const std::string& value)
{
	size_t atPos = value.find('@');
	if (atPos == std::string::npos || atPos == 0)
		return false;

	size_t dotPos = value.find('.', atPos);
	if (dotPos == std::string::npos || dotPos == value.length() - 1)
		return false;

	return true;
}

void Request::fillResponse(Response& response, int statusCode, const std::string& body)
{
	response.setStatus(statusCode);
	//std::cout << "response.getStatusMessage() : "<< response.getStatusMessage() << std::endl;
	//response.setResponse(response.formatResponse());
	//std::cout << "response.getResponse() : "<< response.getResponse() << std::endl;
	response.setBody(body);
	//std::cout << "response.getBody() : "<< response.getBody() << std::endl;
	response.setHeaders(this->getHeaders());
	response.setHttpVersion(this->getHttpVersion());
	//std::cout << "response.getHttpVersion() : " << response.getHttpVersion() << std::endl;
	_response = response;
}

// filename="....."
std::string Request::getFilename() const
{
    std::string filename;
    size_t pos = _body.find("filename=\""); //std::string::size_type pos
	// std::cout << ">>>FILENAME POS : " << pos << std::endl; //DEBUG

    if (pos != std::string::npos)
    {
        //found
        pos += 10; //skip filename="
        size_t endPos = _body.find("\"", pos);//start find frm pos
		
		// std::cout << ">>>FILENAME ENDPOS : " << pos << std::endl; //DEBUG

        if (endPos != std::string::npos)
		    filename = _body.substr(pos, endPos - pos);
		// std::cout << ">>>FILENAME  : " << filename << std::endl; //DEBUG
    }
    return filename;
}

void Request::setServer(Server& server) {
    _server = server;
}

bool Request::isBodySizeValid() const {
    size_t maxSize = 1024 * 1024; // 1MB par défaut
    
    if (_currentLocation && !_currentLocation->getClientMaxBodySize().empty()) {
        maxSize = convertSizeToBytes(_currentLocation->getClientMaxBodySize());
    } else if (!_server.getClientMaxBodySize().empty()) {
        maxSize = convertSizeToBytes(_server.getClientMaxBodySize());
    }
    std::cout << "👻 maxSize: " << maxSize << std::endl;
    std::cout << "👻 _body.length(): " << _body.length() << std::endl;
    return maxSize >= _body.length();
}

bool Request::isMethodAllowed() const {
	if (_currentLocation) {
		std::vector<std::string> allowedMethods = _currentLocation->getMethods();
		std::cout << "👻 allowedMethods: " << allowedMethods.size() << std::endl;
		return std::find(allowedMethods.begin(), allowedMethods.end(), _method) != allowedMethods.end() ;
	}
	return false;
}

bool Request::isContentLengthValid() const {
    std::string contentLength = getHeader("Content-Length");
    if (contentLength.empty()) return true; // Pas de Content-Length, on continue
    
    size_t declaredSize = static_cast<size_t>(atoi(contentLength.c_str()));
    size_t maxSize = 1024 * 1024; // 1MB par défaut
    
    if (_currentLocation && !_currentLocation->getClientMaxBodySize().empty()) {
        maxSize = convertSizeToBytes(_currentLocation->getClientMaxBodySize());
    } else if (!_server.getClientMaxBodySize().empty()) {
        maxSize = convertSizeToBytes(_server.getClientMaxBodySize());
    }
    
    return declaredSize <= maxSize;
}

void Request::parseChunkedBody() {
    if (_isChunked) {
        size_t pos = 0;
        std::string decodedBody;
        
        while (pos < _request.length()) {
            // Trouver la fin de la ligne (taille du chunk)
            size_t endOfLine = _request.find("\r\n", pos);
            if (endOfLine == std::string::npos) break;
            
            // Lire la taille du chunk (en hexadécimal)
            std::string chunkSizeStr = _request.substr(pos, endOfLine - pos);
            size_t chunkSize = hexToSizeT(chunkSizeStr);
            
            if (chunkSize == 0) break; // Fin du transfert
            
            // Ajouter le contenu du chunk
            pos = endOfLine + 2; // Passer après \r\n
            decodedBody += _request.substr(pos, chunkSize);
            pos += chunkSize + 2; // Passer après le chunk et son \r\n
        }
        
        _body = decodedBody;
    }
}

bool Request::isChunked() const { return _isChunked; }


