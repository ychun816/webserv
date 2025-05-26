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
	// Ne continuer que si la requête est valide
	if (_method.empty() || _uri.empty() || _httpVersion.empty()) {
		return;
	}
	setPathQueryString();
	parseQuery();
	_currentLocation = _server.getCurrentLocation(_path);
	if (_currentLocation) {
		std::vector<std::string> methods = _currentLocation->getMethods();
		std::cout << "Méthodes autorisées pour cette location : ";
		for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); ++it) {
			std::cout << *it << " ";
		}
		std::cout << std::endl;
	}
}

Request::~Request()
{
}

void Request::handleResponse()
{
	if (!isContentLengthValid()) {
		Response response(*this);
		std::cout << "👻 Content-Length is not valid" << std::endl;
            // std::string error_response = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\n\r\n";
            // send(_server.getSocketFd(), error_response.c_str(), error_response.size(), 0);
		response.setStatus(413); // Payload Too Large
		return;
	}

    if (!isContentLengthValid()) {
        std::cout << "👻 Content-Length is not valid" << std::endl;
        if (!errorPageExist(413)) {
			response.setStatusMessage(response.getStatusMessage(413));
            buildErrorPageHtml(413, response);
        } else {
            openErrorPage(413, response);
        }
    }
    else if (!isMethodAllowed()) {
        std::cout << "👻 Method not allowed" << std::endl;
        if (!errorPageExist(405)) {
			std::cout << "👻 Error page not exist" << std::endl;
			response.setStatusMessage(response.getStatusMessage(405));
            buildErrorPageHtml(405, response);
        } else {
			std::cout << "👻 Error page exist" << std::endl;
            openErrorPage(405, response);
        }
    }
    else {
        Config* config = Config::getInstance();
        if (config) {
            Server* appropriateServer = config->findServerByLocation(_path, _server.getPort());
            if (appropriateServer)
                _server = *appropriateServer;
        }
        _server.executeMethods(*this, response);
    }
	// std::map<size_t, std::string> redir = getCurrentLocation()->getRedirections();
	// if (!redir.empty())
	// {
	// 	std::map<size_t, std::string>::iterator it = redir.begin();
	// 	std::ifstream file(("./www/simplesite/index.html"));

	// 	std::stringstream buffer;
	// 	buffer << file.rdbuf();
	// 	file.close();

	// 	// Définir le corps de la réponse avec le contenu du fichier
	// 	fillResponse(response, it->first, buffer.str());
	// 	std::map<std::string, std::string> headers;
	// 	headers["Content-Type"] = "text/html";
	// 	response.setHeaders(headers);
	// 	response.setResponse(response.formatResponse());
	// }
	// else
    response.setResponse(response.formatResponse());
    std::cout << BLUE << "Sending response: [" << response.getResponse() << "]" << RESET << std::endl;

}

void Request::openErrorPage(size_t code, Response& response)
{
    response.setStatus(code);

    std::map<std::string, std::string> headers = this->getHeaders();
    Location* loc = _server.getCurrentLocation(_path);

    // Vérifier si loc est NULL avant d'accéder à ses méthodes
    if (loc) {
        std::cout << "loc->getRedirections().size() : " << loc->getRedirections().find(301)->second.c_str() << std::endl;
        std::cout << "loc->getErrorPage().find(code)->second.c_str() : " << loc->getErrorPage().find(code)->second.c_str() << std::endl;
        if (loc->getErrorPage().find(code) != loc->getErrorPage().end())
            _uri = loc->getErrorPage().find(code)->second.c_str();
        else
            _uri = _server.getErrorPages().find(code)->second.c_str();
    } else {
        // Si pas de location trouvée, utiliser les pages d'erreur du serveur
        _uri = _server.getErrorPages().find(code)->second.c_str();
    }

    headers["Content-Type"] = "text/html";
    _method = "GET";
    _path = _uri;
    response.setHeaders(headers);

    Config* config = Config::getInstance();
    if (config) {
        Server* appropriateServer = config->findServerByLocation(_path, _server.getPort());
        if (appropriateServer)
            _server = *appropriateServer;
    }
    _server.executeMethods(*this, response);
    _response = response;
}


void Request::parseRequest()
{
    if (_request.empty()) {
        std::cout << "👻 Request is empty" << std::endl;
        Response response(*this);
        buildErrorPageHtml(400, response);
        _method = ""; // Marquer la requête comme invalide
        return;
    }

    std::stringstream ss(_request);
    std::string line;

    // Parse la première ligne (méthode, URL, version)
    if (std::getline(ss, line) && !line.empty()) {
        parseRequestLine(line);
        if (_method.empty() || _uri.empty() || _httpVersion.empty()) {
            std::cout << "👻 Invalid request line - missing method, URI or HTTP version" << std::endl;
            Response response(*this);
            buildErrorPageHtml(400, response);
            _method = ""; // Marquer la requête comme invalide
            return;
        }
        std::cout << "method: " << _method << std::endl;
        std::cout << "uri: " << _uri << std::endl;
        std::cout << "http version: " << _httpVersion << std::endl;
    } else {
        std::cout << "👻 Empty request line" << std::endl;
        Response response(*this);
        buildErrorPageHtml(400, response);
        _method = ""; // Marquer la requête comme invalide
        return;
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

void Request::fillRedirection(Response& response, int statusCode, const std::string& location)
{
	response.setStatus(statusCode);
	response.setLocation(location);
	//std::cout << "response.getBody() : "<< response.getBody() << std::endl;
	response.setHeaders(this->getHeaders());
	response.setHttpVersion(this->getHttpVersion());
	//std::cout << "response.getHttpVersion() : " << response.getHttpVersion() << std::endl;
	_response = response;
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
	std::cout << "👻 verif :" << (maxSize >= _body.length()) << std::endl;
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
