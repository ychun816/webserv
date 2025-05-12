#include "../../includes/server/Request.hpp"
#include "../../includes/server/Response.hpp"
#include "../../includes/methods/AMethods.hpp"
#include "../../includes/methods/Get.hpp"
#include <sstream>
#include <iostream>

Request::Request(std::string request) :
	_response(),
	_request(request),
	_method(""),
	_uri(""),
	_httpVersion(""),
	//_queryString(""),
	_path(""),
	_headers(),
	_body("")
{
	parseRequest();
	setPathQueryString();
	parseQuery();
}

Request::~Request()
{

}

void Request::handleResponse()
{
	Response response(*this);
	Server server;

	// response.setStatus(404);
	// response.setResponse("NO");
	// response.setStatusMessage("OK");
	// response.setBody("<html><head><style>body{font-family:Arial,sans-serif;background-color:#f0f0f0;display:flex;justify-content:center;align-items:center;height:100vh;margin:0}h1{color:#333;text-align:center;padding:20px;background-color:#fff;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.1)}</style></head><body><h1>404 ERROR</h1></body></html>");
	// response.setHeaders(this->getHeaders());
	// response.setHttpVersion(this->getHttpVersion());
	// // _response = response.formatResponse();
	// std::cout << response.formatResponse() << std::endl;
	// _response = response;

	executeMethods(*this, response, server);
	response.setResponse(response.formatResponse());
}

void	Request::executeMethods(Request& request, Response& response, Server& server)
{
	AMethods*	method = NULL;
	if (request.getMethod() == "GET")
		method = new Get();
	// else if (request.getMethod() == "POST")
	// 	method = new Post();
	// else if (request.getMethod() == "DELETE")
	// 	method = new Delete();
	else
	{
		response.setStatus(405);
		response.formatResponse();
		return;
	}
	method->process(request, response, server);
	delete method;
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
			line = line.substr(0, line.length() - 1); // Enlever le CR si présent
		parseHeader(line);
	}

	// Le reste est le corps de la requête
	std::string body;
	while (std::getline(ss, line)) {
		body += line + "\n";
	}
	_body = body;
	//handleResponse();

}

void Request::parseRequestLine(const std::string& line)
{
	std::istringstream iss(line);
	iss >> _method >> _uri >> _httpVersion;
	// Supprimer le "\r" si présent dans la version HTTP
	if (!_httpVersion.empty() && _httpVersion[_httpVersion.length() - 1] == '\r')
		_httpVersion = _httpVersion.substr(0, _httpVersion.length() - 1);
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

		// Exemple de validation pour certains paramètres courants
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

//get filename (added for POST)
//filename="....."
std::string Request::getFilename() const
{
    std::string filename;
    size_t pos = _body.find("filename=\""); //std::string::size_type pos

    if (pos != std::string::npos)
    {
        //found 
        pos += 10; //skip filename="
        size_t endPos = _body.find("\"", pos);//start find frm pos        
        if (endPos != std::string::npos)
            filename = _body.substr(endPos - pos);  
    }
    return filename;
}