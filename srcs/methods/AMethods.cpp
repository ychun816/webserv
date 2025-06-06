#include "../../includes/methods/AMethods.hpp"

AMethods::AMethods() {}
AMethods::~AMethods() {}

std::string	to_Lower(const std::string& path)
{
	std::string lower_path = path;

	for (size_t i = 0; i < lower_path.length(); i++)
		lower_path[i] = tolower(lower_path[i]);
	return (lower_path);
}

// Normalize the path by adjusting the '/' and removing duplicates
std::string	normalizePath(const std::string& path)
{
	std::string	normalize_path = path;

	for (size_t i = 0; i < normalize_path.length(); i++)
	{
		if (normalize_path[i] == '/' || normalize_path[i] == '\\')
			normalize_path[i] = PATH_SEPARATOR;
	}
	size_t index = 0;
	while ((index = normalize_path.find(std::string(2, PATH_SEPARATOR), index)) != std::string::npos)
		normalize_path.erase(index, 1);
	return (normalize_path);
}

// Check if the path exists
bool pathExist(Request& request, Server& server)
{
	struct stat buffer;
	std::string root = server.getRoot();
	std::cout << "ROOT : " << root << std::endl;
	std::string path = request.getPath();
	std::cout << "Path : " << path << std::endl;
	std::string finalPath;
	if (root.length() >= 2 && root.substr(0, 2) == "./") {
		finalPath = root + path;
	} else {
		finalPath = "." + root + path;
	}
	request.setAbspath(normalizePath(finalPath));
	std::cout << "Exist path : " << request.getAbspath() << std::endl;
	return (stat(request.getAbspath().c_str(), &buffer) == 0);
}

// Fill the vector of sensitive paths
std::vector<std::string>& getDangerousPath()
{
	static std::vector<std::string> dangerous_path;

	if (dangerous_path.empty()) {
		#ifdef _WIN32
		// Chemins Windows sensibles
		dangerous_path.push_back("c:\\windows");
		dangerous_path.push_back("c:\\windows\\system32");
		dangerous_path.push_back("c:\\boot");
		dangerous_path.push_back("c:\\program files");
		dangerous_path.push_back("c:\\program files (x86)");
		dangerous_path.push_back("c:\\pagefile.sys");
		dangerous_path.push_back("c:\\hiberfil.sys");
		dangerous_path.push_back("c:\\boot.ini");
		dangerous_path.push_back("\\\\");  // Partages réseau UNC
		#else
		// Chemins Unix/Linux sensibles
		dangerous_path.push_back("/etc");
		dangerous_path.push_back("/var");
		dangerous_path.push_back("/bin");
		dangerous_path.push_back("/sbin");
		dangerous_path.push_back("/boot");
		dangerous_path.push_back("/dev");
		dangerous_path.push_back("/lib");
		dangerous_path.push_back("/proc");
		dangerous_path.push_back("/sys");
		dangerous_path.push_back("/root");
		dangerous_path.push_back("/usr");
		dangerous_path.push_back("/opt");
		#endif
	}

	return dangerous_path;
}

// Check if the request does not try to access a secure folder.
bool	isPathSafe(const std::string& path)
{
	std::string normalize_path = normalizePath(path);
	std::string lower_path = to_Lower(normalize_path);

	std::vector<std::string> dangerous_path = getDangerousPath();

	if (lower_path.find("..") != std::string::npos)
		return (false);
	for (size_t i = 0; i < dangerous_path.size(); ++i)
	{
		std::string dang_path = to_Lower(normalizePath(dangerous_path[i]));
		if (lower_path.find(dang_path) == 0)
			return (false);
	}
	return (true);
}

// Check if we have access to the path and if there is no security issues.
bool AMethods::checkPath(Request& request, Server& server, Response& response)
{

	if (!pathExist(request, server))
	{
			std::cout << "Path not found: " << request.getAbspath() << std::endl;
		response.setStatus(404);

		return (false);
	}
	if (!isPathSafe(request.getAbspath()))
	{
		std::cout << "Path not safe: " << request.getAbspath() << std::endl;
		response.setStatus(403);
		return (false);
	}
	return (true);
}

void AMethods::handleError(Response& response)
{
	throw (executeError(response.getStatusMessage(response.getStatus())));
	response.formatResponse();
}

FileType AMethods::getFileType(const std::string& path)
{
	struct stat file_info;

		// Vérification de l'existence et récupération des infos
	if (checkIfCgi(path))
		return TYPE_REGULAR_FILE;
	if (stat(path.c_str(), &file_info) != 0) {
		if (errno == ENOENT) {
			return TYPE_NOT_FOUND;
		} else if (errno == EACCES) {
			return TYPE_NO_PERMISSION;
		} else {
			return TYPE_OTHER;
		}
	}
		// Analyse du type
	if (S_ISDIR(file_info.st_mode))
	{
		DIR* dir = opendir(path.c_str());
		if (dir) {
			closedir(dir);
			return TYPE_DIRECTORY;
		} else {
			return TYPE_NO_PERMISSION;  
		}
	}
	else if (S_ISREG(file_info.st_mode))
		return TYPE_REGULAR_FILE;
	else {
		return TYPE_OTHER;
	}
}

bool AMethods::checkIfCgi(std::string filepath)
{
    std::cout << "=== DEBUG checkIfCgi ===" << std::endl;
    std::cout << "Checking filepath: " << filepath << std::endl;
	std::vector<std::string> cgiExt;
	cgiExt.push_back(".php");
	cgiExt.push_back(".py");
	cgiExt.push_back(".rb");
	cgiExt.push_back(".pl");

	size_t lastDot = filepath.find_last_of('.');
	if (lastDot == std::string::npos){
		std::cout << "No extension found" << std::endl;
		return (false);
	}

	std::string extension = filepath.substr(lastDot);
	std::cout << "Extension found: " << extension << std::endl;

	// Convert to lowercase for comparison
	for (size_t i = 0; i < extension.length(); i++)
		extension[i] = tolower(extension[i]);

	for (size_t i = 0; i < cgiExt.size(); i++)
	{
		if (extension == cgiExt[i])
		{
            std::cout << "Extension matches CGI: " << extension << std::endl;
            // Check if the file exists
            struct stat file_stat;
            if (stat(filepath.c_str(), &file_stat) == 0) {
                std::cout << "File exists and is regular" << std::endl;
                if (S_ISREG(file_stat.st_mode)) {
                    std::cout << "Confirmed as CGI script" << std::endl;
                    return true;
                }
            } else {
                std::cout << "File stat failed: " << strerror(errno) << std::endl;
            }
        }
    }
    std::cout << "Not a CGI script" << std::endl;
	return false;
}