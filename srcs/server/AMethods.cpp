#include "../../includes/server/AMethods.hpp"

AMethods::AMethods() {}

AMethods::AMethods(const AMethods& copy) {}

AMethods&	AMethods::operator=(const AMethods& copy) {return *this;}

AMethods::~AMethods() {}

// Converti le path en minuscules
std::string	to_Lower(const std::string& path)
{
	std::string lower_path = path;

	for (size_t i = 0; i < lower_path.length(); i++)
		lower_path[i] = tolower(lower_path[i]);
	return (lower_path);
}

// Normalise le path en ajustant les '/' et en supprimant les doublons
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

// Verifie si le path existe
bool	pathExist(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

// Rempli le vecteur des path sensibles
std::vector<std::string>&	getDangerousPath(std::vector<std::string> dangerous_path)
{
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
	return (dangerous_path);
}

// Verifie si la requete ne tente pas d'acceder a un dossier securise.
bool	isPathSafe(const std::string& path)
{
	std::string normalize_path = normalizePath(path);
	std::string lower_path = to_Lower(normalize_path);

	std::vector<std::string> dangerous_path = getDangerousPath(dangerous_path);

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

// Recupere le path absolue
std::string get_absolute_path(const std::string& path)
{
	char absolute_path[MAX_PATH_LENGTH];

	#ifdef _WIN32
		DWORD result = GetFullPathNameA(path.c_str(), MAX_PATH_LENGTH, absolute_path, NULL);
		if (result == 0 || result >= MAX_PATH_LENGTH) {
			return "";
		}
	#else
		if (realpath(path.c_str(), absolute_path) == NULL) {
			return "";
		}
	#endif
		return std::string(absolute_path);
}

// Verifie si l'on a bien acces au path et si il n'y a pas d'enjeux de securite.
bool AMethods::checkPath(const std::string& path)
{
	try
	{
		pathExist(path);
		get_absolute_path(path);
		isPathSafe(path);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Invalid path" << std::endl;
		return (false);
	}
	return (true);
}

bool AMethods::isMethodAllowed(const Request& request)
{
	if (request.getMethod() == "GET" || request.getMethod() == "POST" || request.getMethod() == "DELETE")
		return (true);
	return (false);
}

void AMethods::handleMethodNotAllowed(Response& response)
{
	response.setStatus(405);
	response.formatResponse();
	//Ajouter la page d'erreur personnalisee
}

void AMethods::handleError(const std::exception& e, Response& response)
{
	throw (executeError(response.getStatusMessage()));
	response.formatResponse();
}