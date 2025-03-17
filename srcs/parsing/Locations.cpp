#include "../../includes/parsing/Locations.hpp"

Location::Location()
{
}

Location::Location(const Location& other)
{
    this->_path = other._path;
    this->_root = other._root;
    this->_autoindex = other._autoindex;
    this->_cgi = other._cgi;
    this->_client_max_body_size = other._client_max_body_size;
    this->_upload_path = other._upload_path;
    this->_index = other._index;
}


Location::~Location()
{
}

void Location::setPath(const std::string& path)
{
    this->_path = path;
}




Location& Location::operator=(const Location& other)
{
    if (this != &other)
    {
        this->_path = other._path;
        this->_root = other._root;
        this->_autoindex = other._autoindex;
        this->_cgi = other._cgi;
        this->_client_max_body_size = other._client_max_body_size;
        this->_upload_path = other._upload_path;
        this->_index = other._index;
    }
    return (*this);
}