#include "../../includes/server/Request.hpp"
#include "../../includes/server/Response.hpp"
#include "../../includes/server/Server.hpp"
#include "../../includes/utils/Utils.hpp"
#include "../../includes/parsing/Config.hpp"
#include <sstream>
#include <iostream>
#include <algorithm> 
#include <cstdlib> 
#include <fstream>
#include <string>

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
	_isChunked(false),
	_isRedirection(false),
    _havePriority(false),
    _originalRoot(server.getRoot()),
    _errorCode(0)

{
	parseRequest();
	setPathQueryString();
	parseQuery();
	_currentLocation = _server.getCurrentLocation(_path);
	if (_currentLocation) {
        // std::cout << "👻 Current location found: " << _currentLocation->getPath() << std::endl;
        if (_currentLocation->getMethods().empty()) {
            // std::cout << "Aucune méthode autorisée pour cette location, utilisation des méthodes par défaut." << std::endl;
            std::list<std::string> allowMethodsList = _server.getAllowMethods();
            std::vector<std::string> allowMethodsVec(allowMethodsList.begin(), allowMethodsList.end());
            _currentLocation->setMethods(allowMethodsVec);
        }
        else if (_currentLocation->getMethods().size() > 0) {
            // std::cout << "Méthodes autorisées pour cette location: ";
            	std::vector<std::string> methods = _currentLocation->getMethods();
            // for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); ++it) {
                // std::cout << *it << " ";
            // }
        }
        if (!_currentLocation->getRoot().empty()) {
            _server.setRoot(_currentLocation->getRoot());
        }
        if (!_currentLocation->getUploadPath().empty()) {
            _server.setUpload(_currentLocation->getUploadPath());
        }

		// std::cout << std::endl;
	}
    if (_currentLocation && _currentLocation->getReturn().first != 0) {
        setIsRedirection(true);
    }
}

Request::~Request()
{
    _server.setRoot(_originalRoot);
    // std::cout << "Response:" << _response.getResponse() << std::endl;
}

void Request::handleResponse()
{
    Response response(*this);
    if (_request.empty() || _method.empty() || _httpVersion.empty()) {
        response.setStatus(400);
        _errorCode = 400;
        errorHandler(response);
        return;
    }
    else if (_httpVersion != "HTTP/1.1") {
        response.setStatus(505);
        _errorCode = 505;
        errorHandler(response);
        return;
    }
    else if (!isMethodAllowed()) {
        response.setStatus(405);
        _errorCode = 405;
        errorHandler(response);
        return;
    }

    std::string hostHeader = getHeader("Host");
    if (hostHeader.empty()) {
        hostHeader = "127.0.0.1";
    }

    Config* config = Config::getInstance();
    if (config) {
        Server* appropriateServer = config->findServerByHost(hostHeader, _server.getPort());
        if (appropriateServer) {
            _server = *appropriateServer;
        } else {
            Server* defaultServer = config->findServerByHost("", _server.getPort());
            if (defaultServer) {
                _server = *defaultServer;
            } else {
                response.setStatus(400);
                _errorCode = 400;
                errorHandler(response);
                return;
            }
        }
    }

    if (!isContentLengthValid()) {
        response.setStatus(413);
        _errorCode = 413;
        errorHandler(response);
        return;
    }

    if (!isMethodAllowed()) {
        response.setStatus(405);
        _errorCode = 405;
        errorHandler(response);
        return;
    }

    if (config) {
        Server* appropriateServer = config->findServerByLocation(_path, _server.getPort());
        if (appropriateServer)
            _server = *appropriateServer;
    }
    _server.executeMethods(*this, response);
    response.setResponse(response.formatResponse());
    if (response.getStatus() >= 400) {
        _errorCode = response.getStatus();
    }
    if (_errorCode != 0) {
        errorHandler(response);
        return;
    }
    debugString(response.getStatus());
    _response = response;
}

void Request::errorHandler(Response& response)
{
    if (_errorCode != 0) {
        if (!errorPageExist(_errorCode)) {
            buildErrorPageHtml(_errorCode, response);
        } else {
            openErrorPage(_errorCode, response);
        }
    }
}

void Request::openErrorPage(size_t code, Response& response)
{
    response.setStatus(code);
    debugString(response.getStatus());

    std::map<std::string, std::string> headers = this->getHeaders();
    Location* loc = _server.getCurrentLocation(_path);
    std::string errorPagePath;

    if (loc && !loc->getErrorPage().empty()) {
        const std::map<size_t, std::string>& errorPage = loc->getErrorPage(); // Store reference to the map
        std::map<size_t, std::string>::const_iterator it = errorPage.find(code);
        if (it != errorPage.end() && !it->second.empty()) {
            errorPagePath = it->second;
        }
    }

    if (errorPagePath.empty()) {
        std::map<size_t, std::string>::const_iterator it = _server.getErrorPages().find(code);
        if (it != _server.getErrorPages().end() && !it->second.empty()) {
            errorPagePath = it->second;
        }
    }

    if (!errorPagePath.empty()) {

        if (errorPagePath[0] != '/') {
            errorPagePath = "/" + errorPagePath;
        }
        _uri = errorPagePath;
        _path = errorPagePath;

        // std::cout << "👻 Chemin de la page d'erreur: " << _path << std::endl;

        headers["Content-Type"] = "text/html";
        _method = "GET";
        response.setHeaders(headers);

        Config* config = Config::getInstance();
        if (config) {
            Server* appropriateServer = config->findServerByLocation(_path, _server.getPort());
            if (appropriateServer)
                _server = *appropriateServer;
        }
        _server.executeMethods(*this, response);
        response.setStatus(code);
        _response = response;
    } else {
        buildErrorPageHtml(code, response);
    }
    // debugString(response.getStatus());
}

void Request::buildErrorPageHtml(size_t code, Response& response)
{
    response.setStatus(code);

    std::ostringstream oss;
    oss << code;
    std::string codeStr = oss.str();

    response.setBody("<html><body><h1>Error " + codeStr + ": " + response.getStatusMessage(code) + "</h1></body></html>");

    std::map<std::string, std::string> headers = this->getHeaders();
    headers["Content-Type"] = "text/html";
    response.setHeaders(headers);
    response.setResponse(response.formatResponse());
    debugString(response.getStatus());
    _response = response;
}

void Request::parseRequest()
{
    std::stringstream ss(_request);
    // std::cout << "👻 _request: " << _request << std::endl;
    std::string line;
    if (std::getline(ss, line) && !line.empty()) {
        parseRequestLine(line);

    }

    while (std::getline(ss, line) && !line.empty() && line != "\r") {
        if (!line.empty() && line[line.length() - 1] == '\r')
            line = line.substr(0, line.length() - 1);
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
}

void Request::parseRequestLine(const std::string& line)
{
	std::istringstream iss(line);
	iss >> _method >> _uri >> _httpVersion;

	if (!_httpVersion.empty() && _httpVersion[_httpVersion.length() - 1] == '\r')
		_httpVersion = _httpVersion.substr(0, _httpVersion.length() - 1);
}

void Request::parseHeader(const std::string& line)
{
	size_t colonPos = line.find(':');
	if (colonPos != std::string::npos) {
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

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

bool Request::errorPageExist(size_t code)
{
	// std::map<size_t, std::string> errorPages = _currentLocation->getErrorPage().find(code);
	// std::map<size_t, std::string>::iterator it = errorPages.find(code);
	std::map<size_t, std::string> errorPages = _server.getErrorPages();
    std::map<size_t, std::string>locationErrorPages = _currentLocation ? _currentLocation->getErrorPage() : std::map<size_t, std::string>();
    if (_currentLocation && !locationErrorPages.empty())
    {
        // std::cout << "locationErrorPages: " << locationErrorPages.size() << std::endl;
        for (std::map<size_t, std::string>::const_iterator it = locationErrorPages.begin(); it != locationErrorPages.end(); ++it)
        {
            // std::cout << "printing location error pages" << std::endl;
            // std::cout << "Location Error page: " << it->first << " : " << it->second << std::endl;
            if (it->first == code)
            {
                // std::cout << "Location Error page exist: " << it->second << std::endl;
                this->setHavePriority(true);
                return true;
            }
        }
    }
	// std::cout << "errorPages: " << errorPages.size() << std::endl;
	for (std::map<size_t, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
	{
		// std::cout << "printing error pages" << std::endl;
		// std::cout << "Error page: " << it->first << " : " << it->second << std::endl;
		// std::cout << "Error page exist: " << it->second << std::endl;
		// std::cout << "Error page exist: " << it->first << std::endl;
		if (it->first == code)
		{
			// std::cout << "Error page exist: " << it->second << std::endl;
            this->setHavePriority(true);
			return true;
		}
	}
	if (errorPages.find(code) != errorPages.end())
	{
		std::map<size_t, std::string>::const_iterator serverErrorPage = errorPages.find(code);

		if (serverErrorPage != _server.getErrorPages().end())
		{
			// std::cout << "Server Error page: " << serverErrorPage->second << std::endl;
			return true;
		}
	}
	return false;
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
    // std::cout << "👻 fillResponse appelé avec le code: " << statusCode << std::endl;
    response.setStatus(statusCode);
    response.setBody(body);
    response.setHeaders(this->getHeaders());
    response.setHttpVersion(this->getHttpVersion());
    response.setStatusMessage(response.getStatusMessage(statusCode));
    _response = response;
}

std::string Request::getFilename() const
{
    std::string filename;
    size_t pos = _body.find("filename=\"");

    if (pos != std::string::npos)
    {

        pos += 10;
        size_t endPos = _body.find("\"", pos);

        if (endPos != std::string::npos)
		    filename = _body.substr(pos, endPos - pos);
    }
    return filename;
}

void Request::setServer(Server& server) {
    _server = server;
}

bool Request::isBodySizeValid() const {
    size_t maxSize = 1024 * 1024;

    if (_currentLocation && !_currentLocation->getClientMaxBodySize().empty()) {
        maxSize = convertSizeToBytes(_currentLocation->getClientMaxBodySize());
    } else if (!_server.getClientMaxBodySize().empty()) {
        maxSize = convertSizeToBytes(_server.getClientMaxBodySize());
    }
    // std::cout << "👻 maxSize: " << maxSize << std::endl;
    // std::cout << "👻 _body.length(): " << _body.length() << std::endl;
	// std::cout << "👻 verif :" << (maxSize >= _body.length()) << std::endl;
    return maxSize >= _body.length();
}

bool Request::isMethodAllowed() const {
    if (_currentLocation) {
        std::vector<std::string> allowedMethods = _currentLocation->getMethods();
        // Si aucune méthode n'est spécifiée dans la location, utiliser les méthodes par défaut du serveur
        if (allowedMethods.empty()) {
            std::list<std::string> serverMethods = _server.getAllowMethods();
            return std::find(serverMethods.begin(), serverMethods.end(), _method) != serverMethods.end();
        }
        return std::find(allowedMethods.begin(), allowedMethods.end(), _method) != allowedMethods.end();
    }
    else if (!_server.getAllowMethods().empty()) {
        std::list<std::string> allowedMethods = _server.getAllowMethods();
        return std::find(allowedMethods.begin(), allowedMethods.end(), _method) != allowedMethods.end();
    }
    return false;  // Si aucune méthode n'est spécifiée nulle part, toutes les méthodes sont interdites
}

bool Request::isContentLengthValid() const {
    std::string contentLength = getHeader("Content-Length");
    if (contentLength.empty()) return true; 

    size_t declaredSize = static_cast<size_t>(atoi(contentLength.c_str()));
    size_t maxSize = 1024 * 1024; 

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
            size_t endOfLine = _request.find("\r\n", pos);
            if (endOfLine == std::string::npos) break;

            std::string chunkSizeStr = _request.substr(pos, endOfLine - pos);
            size_t chunkSize = hexToSizeT(chunkSizeStr);

            if (chunkSize == 0) break;
            pos = endOfLine + 2;
            decodedBody += _request.substr(pos, chunkSize);
            pos += chunkSize + 2;
        }

        _body = decodedBody;
    }
}

bool Request::isChunked() const { return _isChunked; }
