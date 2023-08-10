#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

std::mutex logMutex;

/*void logCommands(const std::string& clientInfo, const std::string& command)
{
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream logFile("log.txt", std::ios_base::app);
    logFile << "[" << clientInfo << "]: " << command << "\n";
}*/

void handleCommand(int clientSocket, sockaddr_in clientAddress)
{
    char buffer[128] = {0};
    recv(clientSocket, buffer, sizeof(buffer), 0);

    const char prompt[] = "# ";
    send(clientSocket, prompt, sizeof(prompt), 0);

    while (true) {
        char command[128] = {0};
        std::string response;

        recv(clientSocket, command, sizeof(command), 0);

        // Log the captured command
        std::string clientInfo = std::string(inet_ntoa(clientAddress.sin_addr)) + ":" + std::to_string(ntohs(clientAddress.sin_port));
        std::string commandStr = command;
        //logCommands(clientInfo, commandStr);

        commandStr += "\r\n";
        commandStr.erase(commandStr.find_last_not_of("\r\n") + 1);

        std::string commandString = commandStr;
        if (commandString == "sh" || commandString == "shell")
        {
            response = "sh: shell: not found\r\n";
        }
        else if (commandString == "enable")
        {
            response = "sh: enable: not found\r\n";
        }
        else if (commandString == "system")
        {
            response = "sh: system: not found\r\n";
        }
        else if (commandString == "linuxshell")
        {
            response = "sh: linuxshell: not found\r\n";
        }
        else if (commandString == "/bin/busybox wget")
        {
            response = "wget: missing URL\r\n";
            response += "Usage: wget [OPTION]... [URL]...\r\n";
            response += "\r\n";
            response += "Try `wget --help' for more options.\r\n\r\n";
        }
        else if (commandString.find("/bin/busybox ") == 0)
        {
            response = commandStr + ": applet not found\r\n";
        }
        else if (commandString == "exit")
        {
            response = "Goodbye!\r\n";
            send(clientSocket, response.c_str(), response.length(), 0);
            break;
        }
        else
        {
            response = commandStr + ": not found\r\n";
        }

        send(clientSocket, response.c_str(), response.length(), 0);
        send(clientSocket, prompt, sizeof(prompt), 0);
    }

    close(clientSocket);
    std::cout << "[pandora] Connection closed with " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;
}

int main() {
    const char listenAddress[] = "0.0.0.0";
    const int listenPort = 23;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "[pandora] Error creating socket" << std::endl;
        return 1;
    }

    int opt = 1;

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "[pandora] Error setting socket options" << std::endl;
        close(serverSocket);
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(listenAddress);
    serverAddress.sin_port = htons(listenPort);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "[pandora] Error binding socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1)
    {
        std::cerr << "[pandora] Error listening on the socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "The Pandora's Box Has opened!" << std::endl;
    std::cout << "[pandora] Telnet honeypot is listening on " << listenAddress << ":" << listenPort << std::endl;

    try {
        while (true) {
            sockaddr_in clientAddress;
            socklen_t clientAddressSize = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);

            if (clientSocket == -1)
            {
                std::cerr << "[pandora] Error accepting connection" << std::endl;
                continue;
            }

            std::cout << "[pandora] Connection established with " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;

            std::thread clientThread(handleCommand, clientSocket, clientAddress);
            clientThread.detach();
        }
    }

    catch (const std::exception& ex)
    {
        std::cerr << "[pandora] Exception occurred: " << ex.what() << std::endl;
    }

    close(serverSocket);
    return 0;
}
