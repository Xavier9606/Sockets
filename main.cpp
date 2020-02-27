#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

using namespace std;

int main() {
  WSADATA WSAData;
  SOCKET server, client;
  SOCKET clients[10];
  int activeClients=0;
  SOCKADDR_IN serverAddr, clientAddr;

  WSAStartup(MAKEWORD(2, 0), &WSAData);
  server = socket(AF_INET, SOCK_STREAM, 0);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(5555);

  bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
  listen(server, 0);

  char buffer[1024];
  int clientAddrSize = sizeof(clientAddr);

  if ((client = accept(server, (SOCKADDR *) &clientAddr, &clientAddrSize)) != INVALID_SOCKET) {

    cout << "Client connected!" << endl;
    recv(client, buffer, sizeof(buffer), 0);
    cout << "Client says:" << buffer << endl;
    memset(buffer, 0, sizeof(buffer));
    char *hello = "Hello from server!";
    send(client, hello, strlen(hello), 0);

    Sleep(100000);
    closesocket(client);
    cout << "Client disconnected." << endl;
  }
  return 0;
}
