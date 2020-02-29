#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>

WSADATA WSAData;
SOCKET server, client;
int activeClient = 0;
SOCKADDR_IN serverAddr, clientAddr;
bool newConnectionNeeded = true;

struct ClientData {
  SOCKET clientSocket;
  SOCKADDR_IN clientAddr;
  int clientAddrSize = sizeof(clientAddr);
  char buffer[1024];
  int clientId;
};// clients[10];

std::vector<ClientData *> clients;
std::mutex m;


void newClient() {
  clients.push_back(new ClientData);
  ClientData *currentClient = clients[activeClient];
  if ((currentClient->clientSocket = accept(server, (SOCKADDR *) &currentClient->clientAddr,
                                            &currentClient->clientAddrSize)) != INVALID_SOCKET) {
    m.lock();
    newConnectionNeeded = true;
    currentClient->clientId = activeClient;
    std::string msg = " ";
    msg = std::to_string(currentClient->clientId);
    send(currentClient->clientSocket, (char *) msg.c_str(), msg.size(), 0);
    activeClient++;
    std::cout << "Client " << currentClient->clientId << " connected!" << std::endl;
    m.unlock();
    while (true) {
      //if(clients[clientId].clientSocket == INVALID_SOCKET){return;}
      //Sleep(500);
      if (recv(currentClient->clientSocket, currentClient->buffer, sizeof(currentClient->buffer), 0) <= 0) {
        return;
      }

      m.lock();
      std::cout << "Client " << currentClient->clientId << " says: " << currentClient->buffer << std::endl
                << std::flush;
      msg = "Client";
      msg += std::to_string(currentClient->clientId);
      msg += " says: ";
      msg += currentClient->buffer;
      //std::cout << msg;
      memset(currentClient->buffer, 0, sizeof(currentClient->buffer));
      m.unlock();

      for (int i = 0; i < activeClient; ++i) {
        if (i == currentClient->clientId) {
          continue;
        }
        send(clients[i]->clientSocket, (char *) msg.c_str(), sizeof(msg), 0);
      }

    }
  }
}

void connectionMaster() {
  while (true) {
    Sleep(10);
    if (newConnectionNeeded) {
      std::thread clientListener(newClient);
      clientListener.detach();
      m.lock();
      newConnectionNeeded = false;
      m.unlock();
    }
  }
}


int main() {

  WSAStartup(MAKEWORD(2, 0), &WSAData);
  server = socket(AF_INET, SOCK_STREAM, 0);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(5555);

  bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
  listen(server, 0);


  std::thread connectorMasterLoop(connectionMaster);
  connectorMasterLoop.join();

  return 0;
}
