#include "../../includes/server/Server.hpp"
#include "../../includes/methods/Get.hpp" //added to exec methods
#include "../../includes/methods/Post.hpp" //added to exec methods
#include "../../includes/methods/Delete.hpp" //added to exec methods

// Constructors
Server::Server() : _epoll_fd(-1), _configFile(""), _socketFd(-1), _port(0), _host(""), _root(""), _index(""), _errorPage(""), _cgi(""), _upload(""), _clientMaxBodySize(""), _allowMethods(std::list<std::string>()) {
        // Ne pas appeler ces méthodes automatiquement, elles seront appelées explicitement

}

// Constructeur de copie
Server::Server(const Server& other) {
    _configFile = other._configFile;
    _socketFd = other._socketFd;
    _port = other._port;
    _host = other._host;
    _root = other._root;
    _index = other._index;
    _errorPage = other._errorPage;
    _cgi = other._cgi;
    _upload = other._upload;
    _clientMaxBodySize = other._clientMaxBodySize;
    _allowMethods = other._allowMethods;
    _connexions = other._connexions;
    _address = other._address;
    _errorPages = other._errorPages;
    _locations = other._locations;
}

// Destructor
Server::~Server() { std::cout << this->_configFile << std::endl; }

void    Server::createSocket() {
    if ((this->_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Creation de la socket, retourne son FD
                throw std::runtime_error("Erreur lors de la création du socket");
        // Sert a pouvoir reutiliser le port (ici 8080) et evite donc l'erreur d'adderss already in use
        const int enable = 1;
        if (setsockopt(this->_socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
                std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;

    // Permet à plusieurs processus de se lier au même port
        if (setsockopt(this->_socketFd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
                std::cerr << "setsockopt(SO_REUSEPORT) failed" << std::endl;

        setNonBlocking(); // Passage en mode non-Bloquant
        std::memset(&this->_address, 0, sizeof(this->_address));
        // PORT
        this->_address.sin_port = htons(this->_port);
        // IPV4 INTERNET PROTOCOL
        this->_address.sin_family = AF_INET;
        // _adress IP DE LA SOCKET (INADDR _ANY = l'os se debrouille)
        this->_address.sin_addr.s_addr = htonl(INADDR_ANY);
         // Bind la socket a l'interface reseau, la relie. On peut donc s'y connecter en  utilisant le bon port et recevoir des connexions entrantes. En gros c'est son adresse
        if (bind(this->_socketFd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0)
        {
                close(this->_socketFd);
                throw Server::configError("Bind failed");
        }
        else
                std::cout << "Bind Succes" << std::endl;
}

void    Server::configSocket() {
        // Pour un serveur web, une file d'attente de 10 à 128 connexions est généralement recommandée
        // SOMAXCONN est la valeur maximale recommandée par le système (généralement 128)
        if (listen(this->_socketFd, SOMAXCONN) < 0)
                throw Server::configError("Échec de l'écoute sur le socket");

}

void    Server::runServer() {
        // On va attendre une nouvelle connexion entrante avec accept(). acept cree un nouveau socket
        // specifique a cette connexion et return son FD

        struct  epoll_event event, events[MAX_EVENTS];

        // Creer l'instance epoll. le parametre est obsolete donc mettre ce qu'on veut. Preferer Epoll_create1() mais interdit dan sle projet 42 webserv
        if ((this->_epoll_fd = epoll_create(1)) < 0)
                throw Server::configError("Epoll Error");
        else
        {
                std::cout << "Epoll crée avec succes with fd: " << this->_epoll_fd << std::endl;
                event.events = EPOLLIN;
                event.data.fd = this->_socketFd;
                if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, this->_socketFd, &event) == -1)
                        throw Server::configError("Fail to add Socket to epoll (epoll_ctl)");
        }
        std::cout << "En attente de connexions sur le port " << this->_port << std::endl;
    while (true) {
        int event_count = epoll_wait(this->_epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            throw Server::configError("epoll_wait");
            break;
        }
        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == this->_socketFd) {
                handleNewConnection();
            } else {
                // Gestion des connexions clients existantes
                char buffer[1024] = {0};
                int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
                if (bytes_read > 0) {
                    std::string request(buffer, bytes_read);
                    Request req(request, *this);
                    req.handleResponse();
                    send(events[i].data.fd, req.getResponse().c_str(), req.getResponse().size(), 0);
                    
                    // Vérifier si la connexion doit être maintenue (keep-alive)
                    if (req.getHeader("Connection") != "keep-alive") {
                        close(events[i].data.fd);
                        epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        this->_connexions.pop_back();
                    }
                } else {
                    // Client déconnecté
                    close(events[i].data.fd);
                    epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    this->_connexions.pop_back();
                }
            }
        }
    }
    close(this->_socketFd);
    close(this->_epoll_fd);
}

void Server::setNonBlocking()
{
        int flags = fcntl(this->_socketFd, F_GETFL, 0); // Recupere  le flag actuel de la socket
    if (flags == -1) {
        std::cerr << "Erreur fcntl(F_GETFL)" << std::endl;
    }
        std::cout << "Flag a la creation de la socket: " << flags << std::endl;
    if (fcntl(this->_socketFd, F_SETFL, flags | O_NONBLOCK) == -1) { // Ajoute le flag 0_NONBLOCK (devient non bloquant)
        std::cerr << "Erreur fcntl(F_SETFL)" << std::endl;
    }
        std::cout << "Flag a la creation de la socket: " << flags << std::endl;

}
void Server::init(std::string &configFile)
{
    this->_configFile = configFile;
        createSocket();
        configSocket();
        runServer();
}

void Server::pushLocation(const Location& location)
{
        this->_locations.push_back(location);
}

void Server::setEpollFd(int epoll_fd)
{
    this->_epoll_fd = epoll_fd;
}

void Server::handleNewConnection()
{
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int client_fd = accept(this->_socketFd, (struct sockaddr *)&client_addr, &client_addrlen);
    
    if (client_fd < 0) {
        std::cerr << "Accept echoué" << std::endl;
        return;
    }

    std::cout << "Nouvelle connexion client acceptée: " << client_fd << std::endl;
    this->_connexions.push_back(client_fd);

    // Ajouter le socket client à epoll
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client_fd;
    if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
        std::cerr << "Erreur lors de l'ajout du client à epoll" << std::endl;
        close(client_fd);
        this->_connexions.pop_back();
        return;
    }
}

// Operators
Server & Server::operator=(const Server &assign)
{
    if (this != &assign) {
        _configFile = assign._configFile;
        _socketFd = assign._socketFd;
        _port = assign._port;
        _host = assign._host;
        _root = assign._root;
        _index = assign._index;
        _errorPage = assign._errorPage;
        _cgi = assign._cgi;
        _upload = assign._upload;
        _clientMaxBodySize = assign._clientMaxBodySize;
        _allowMethods = assign._allowMethods;
        _connexions = assign._connexions;
        _address = assign._address;
                _locations = assign._locations;
    }
    return *this;
}


//ADDED
/**
 * @note cannot use switch case (use int) /  getMethod() return string X ! 
 * @note Get(). / Post(). / Delete(). -> temp create an object first to run exec()
 * @note *this -> current instance of that class (currently inside server class ) => passing the current Server instance by reference?? //TO CHECK!
 */

void Server::executeMethods(Request& request, Response& response)
{
	std::string method = request.getMethod();

        AMethods*       exec = NULL;
	if (method == "GET")
		exec = new Get();
	else if (method == "POST")
		exec = new Post();
	else if (method == "DELETE")
		exec = new Delete();
	else
		response.setStatus(405); //method not allowed
        exec->process(request, response, *this);
}