#include "../../includes/parsing/Locations.hpp"

Location::Location() : _path(""), _root(""), _autoindex(""), _cgi_ext(""), _client_max_body_size(""), _upload_path(""), _index(""), _error_page(""), _methods(std::vector<std::string>())
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
}


Location::~Location()
{
}


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
    }
    return (*this);
}