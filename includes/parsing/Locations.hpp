#pragma once

#include <string>
class Location
{
private:
    std::string _path;
    std::string _root;
    std::string _autoindex;
    std::string _cgi;
    std::string _client_max_body_size;
    std::string _upload_path;
    std::string _index;
    std::string _error_page;
    std::string _methods;

public:
    Location();
    Location(const Location& other);
    ~Location();

    // Setters
    void setPath(const std::string& path);
    void setRoot(const std::string& root);
    void setAutoindex(const std::string& autoindex);
    void setCgi(const std::string& cgi);
    void setClientMaxBodySize(const std::string& client_max_body_size);
    void setUploadPath(const std::string& upload_path);
    void setIndex(const std::string& index);
    void setErrorPage(const std::string& error_page);
    void setMethods(const std::string& methods);
    void setReturn(const std::string& Return);
    void setCgiExt(const std::string& cgi_ext);
    // Getters
    std::string getPath()  const;
    std::string getRoot() const;
    std::string getAutoindex() const;
    std::string getCgi() const;
    std::string getClientMaxBodySize() const;
    std::string getUploadPath() const;
    std::string getIndex() const;
    std::string getReturn() const;
    std::string getCgiExt() const;
    // Operators
    Location& operator=(const Location& other);
};

