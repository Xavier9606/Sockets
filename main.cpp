#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <thread>
#include <mutex>

WSADATA WSAData;
SOCKET server, client;
int activeClient = 0;
SOCKADDR_IN serverAddr, clientAddr;
bool newConnectionNeeded = true;

struct clientData {
  SOCKET clientSocket;
  SOCKADDR_IN clientAddr;
  int clientAddrSize = sizeof(clientAddr);
  char buffer[1024];
} clients[10];

std::mutex m;


void newClient() {
  //SOCKET clientSocket;
  if ((clients[activeClient].clientSocket = accept(server, (SOCKADDR *) &clients[activeClient].clientAddr,
                                                   &clients[activeClient].clientAddrSize)) != INVALID_SOCKET) {
    m.lock();
    newConnectionNeeded = true;
    int clientId = activeClient;
    std::string msg = " ";
    msg = std::to_string(clientId);
    send(clients[clientId].clientSocket, (char *) msg.c_str(), msg.size(), 0);
    activeClient++;
    std::cout << "Client " << clientId << " connected!" << std::endl;
    m.unlock();
    while (true) {
      //if(clients[clientId].clientSocket == INVALID_SOCKET){return;}
      //Sleep(500);
      if (recv(clients[clientId].clientSocket, clients[clientId].buffer, sizeof(clients[clientId].buffer), 0) <= 0) {
        return;
      }

      m.lock();
      std::cout << std::endl << "Client " << clientId << " says: " << clients[clientId].buffer << std::flush;
      msg = "Client";
      msg += std::to_string(clientId);
      msg += " says: ";
      msg += clients[clientId].buffer;
      //std::cout << msg;
      memset(clients[clientId].buffer, 0, sizeof(clients[clientId].buffer));
      m.unlock();

      for (int i = 0; i < activeClient; ++i) {
        if (i == clientId) {
          continue;
        }
        send(clients[i].clientSocket, (char *) msg.c_str(), sizeof(msg), 0);
      }
      //Sleep(1000);
    }
  }
}

void connectionMaster() {
  while (true) {
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


  // Sleep(1000000);

  return 0;
}
