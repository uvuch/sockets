#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int errorMessage(const char* msg);
int successMessage(const char* msg);
void dump(void* p, size_t bytes);

const socklen_t SOCKADDR_SIZE = sizeof(struct sockaddr);
const int PENDING_CONNECTIONS_SIZE = 1024;

int main(int argc, char **argv) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return -1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        return errorMessage("Could not open server socket.");
    else successMessage("Server socket opened.");

    
    memset(&server_addr, 0, SOCKADDR_SIZE); /* Clear structure */        
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (const sockaddr*) &server_addr, SOCKADDR_SIZE) == -1) {
        close(server_socket);
        return errorMessage("Could not bind server socket.");
    }
    else successMessage("Server socket bound.");

    if (listen(server_socket, PENDING_CONNECTIONS_SIZE) == -1) {
        close(server_socket);
        return errorMessage("Cannot listen to server socket.");
    }
    else successMessage("Server socket starts to listen.");

    const int READ_BUFFER_SIZE = 1024;
    char read_buf[READ_BUFFER_SIZE];
    size_t bytes_received;
    int addr_size;
    
    while(1) {
        client_socket = accept(server_socket, (struct sockaddr*) &client_addr, (socklen_t*) &addr_size);
        if (client_socket == -1) {
            shutdown(server_socket, SHUT_RDWR);
            close(server_socket);
            return errorMessage("Coudn't accept an incomming connection request.");
        } else {
            char str_client_address[16];

            successMessage("New client connection accepted.");
            inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), str_client_address, 16);
            std::cout << '\t' << str_client_address << ':' << ntohs(client_addr.sin_port) << std::endl;

            while(bytes_received = recv(client_socket, read_buf, READ_BUFFER_SIZE, 0)) {
                if (bytes_received == -1)
                    errorMessage("Couldn't read from the client socket.");
                else {
                    successMessage("Message received from the client: ");
                    std::cout << bytes_received << " bytes:\t" << read_buf << std::endl;

                    send(client_socket, "OK", 2, 0);
                }
            }
        }
    }

    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    close(server_socket);

    successMessage("Server shut down.");
    return 0;
}

int errorMessage(const char* msg) {
    std::cout << "\033[31;1;4mError:\033[0m ";
    std::cout << msg << std::endl;
    std::cout << '\t' << errno << ": " << strerror(errno) << std::endl;

    return 1;
}

int successMessage(const char* msg) {
    std::cout << "> \033[32m" << msg << "\033[0m" << std::endl;

    return 0;
}

void dump(void* p, size_t bytes) {
    for(int i = 0; i < bytes; i++) {
        u_char byte = ((u_char*) p)[i];
        std::cout << std::setw(2) << std::setfill('0');
        std::cout << std::hex << (u_int) byte << " ";
    }
    std::cout << std::endl;
}