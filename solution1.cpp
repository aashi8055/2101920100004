#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <winsock2.h>  
#pragma comment(lib, "ws2_32.lib")  

#define PORT 9876
#define BUFFER_SIZE 4096
#define WINDOW_SIZE 10

std::vector<int> numbers;

int generate_random_number() {
    return rand() % 100 + 1;
}

std::string handle_request(const std::string &request) {
    std::string response;

    if (request.find("GET /numbers") != std::string::npos) {
       
        int new_number = generate_random_number();

        
        if (std::find(numbers.begin(), numbers.end(), new_number) == numbers.end()) {
            numbers.push_back(new_number);
        }

        if (numbers.size() > WINDOW_SIZE) {
            numbers.erase(numbers.begin());
        }

        
        double average = std::accumulate(numbers.begin(), numbers.end(), 0.0) / numbers.size();

    
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: application/json\r\n"
            << "Connection: close\r\n\r\n"
            << "{\"numbers\":[";

        for (size_t i = 0; i < numbers.size(); ++i) {
            oss << numbers[i];
            if (i < numbers.size() - 1) oss << ",";
        }

        oss << "],\"avg\":" << average << "}";

        response = oss.str();
    } else {
        response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nInvalid Request";
    }

    return response;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);

   
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

   
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

   
    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started at http://localhost:" << PORT << std::endl;

    while (true) {
    
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        char buffer[BUFFER_SIZE] = {0};
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesReceived > 0) {
            std::string request(buffer);
            std::string response = handle_request(request);

            send(clientSocket, response.c_str(), response.size(), 0);
        }

        closesocket(clientSocket);
    }

   
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
