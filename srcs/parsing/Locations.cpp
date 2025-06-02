#include "../../includes/parsing/Locations.hpp"

#include <iostream>

Location::Location() : _path(""), _root(""), _autoindex(""), _cgi_ext(""), _client_max_body_size(""), _upload_path(""), _index(""), _error_page(std::map<size_t, std::string>()), _methods(std::vector<std::string>()), _return(std::make_pair(0, ""))
{
}

Location::Location(const Location& other)
{
    this->_path = other._path;
    this->_root = other._root;
    this->_autoindex = other._autoindex;
    this->_cgi_ext = other._cgi_ext;
    this->_client_max_body_size = other._client_max_body_size;
    this->_upload_path = other._upload_path;
    this->_index = other._index;
    this->_error_page.clear();
    for (std::map<size_t, std::string>::const_iterator it = other._error_page.begin(); it != other._error_page.end(); ++it) {
        this->_error_page[it->first] = std::string(it->second);
    }
    this->_methods = other._methods;
    this->_return = other._return;
}


Location::~Location(){}


Location& Location::operator=(const Location& other)
{
    if (this != &other)
    {
        this->_path = other._path;
        this->_root = other._root;
        this->_autoindex = other._autoindex;
        this->_cgi_ext = other._cgi_ext;
        this->_client_max_body_size = other._client_max_body_size;
        this->_upload_path = other._upload_path;
        this->_index = other._index;
        this->_error_page = other._error_page;
        this->_methods = other._methods;
        this->_return = other._return;
    }
    return (*this);
}