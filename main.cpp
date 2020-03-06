#include <atomic>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <utility>

//#define ALSO_LOG_TO_FILE
const char *FILEPATH = "C:/Users/Administrator/Desktop/FileTest.txt";

WSADATA WSAData;
SOCKET server;
int activeClient = 0;
SOCKADDR_IN serverAddr;

bool newConnectionNeeded = true;
std::atomic<bool> queueEmpty{true};
FILE *file;


struct ClientData {
  SOCKET clientSocket;
  SOCKADDR_IN clientAddr;
  int clientAddrSize = sizeof(clientAddr);
  char buffer[1024];
  int clientId;
};

struct Massage {
  int from = -1;
  char *msg = nullptr;
  int msgSize = 0;
  int destClient = -1;
};

std::queue<Massage *> MsgQueue;
std::vector<ClientData *> clients;
std::mutex clientLock, coutLock, queueLock, connBoolLock;
std::condition_variable CVqueueIsNotEmpty;

void Client() {
  clientLock.lock();
  clients.push_back(new ClientData);
  ClientData *currentClient = clients[activeClient];
  clientLock.unlock();
  if ((currentClient->clientSocket = accept(server, (SOCKADDR *) &currentClient->clientAddr,
                                            &currentClient->clientAddrSize)) != INVALID_SOCKET) {
  	
    memset(currentClient->buffer, 0, sizeof(currentClient->buffer));
    connBoolLock.lock();
    newConnectionNeeded = true;
    connBoolLock.unlock();

    clientLock.lock();
    currentClient->clientId = activeClient;
    clientLock.unlock();

    std::string msg = " ";
    msg = std::to_string(currentClient->clientId);
    send(currentClient->clientSocket, (char *) msg.c_str(), msg.size(), 0);
    activeClient++;
    coutLock.lock();
    std::cout << "Client " << currentClient->clientId << " connected!" << std::endl;
    coutLock.unlock();

    while (true) {
      if (recv(currentClient->clientSocket, currentClient->buffer, sizeof(currentClient->buffer), 0) <= 0) {
        return;
      }
      msg = "Client";
      msg += std::to_string(currentClient->clientId);
      msg += " says: ";
      msg += currentClient->buffer;
      coutLock.lock();
      std::cout << msg << std::endl << std::flush;
      coutLock.unlock();

      memset(currentClient->buffer, 0, sizeof(currentClient->buffer));

      queueLock.lock();
      MsgQueue.push(new Massage{currentClient->clientId, (char *) msg.c_str(), (int)msg.size()});
      queueEmpty = false;
      queueLock.unlock();
    }
  }
}

void massageChanneling() {
  Massage *currentMassage = nullptr;
  while (true) {

    while (1 != MsgQueue.empty()) {
      queueLock.lock();
      if (1 == MsgQueue.empty()) {
        queueLock.unlock();
        break;
      }
      currentMassage = MsgQueue.front();
      MsgQueue.pop();
      //std::cout << MsgQueue.size() << ::std::endl;
      queueLock.unlock();
      if (-1 == currentMassage->destClient) {
        for (auto k : clients) {
          if (k->clientId == currentMassage->from) continue;
#ifdef ALSO_LOG_TO_FILE
          fprintf(file, "%s\n", currentMassage->msg);
          fflush(file);
#endif
          send(k->clientSocket, currentMassage->msg, currentMassage->msgSize, 0);
        }
      } else {
        for (auto j : clients) {
          if (j->clientId == currentMassage->destClient) {
#ifdef ALSO_LOG_TO_FILE
            fprintf(file, "%s\n", currentMassage->msg);
            fflush(file);
#endif
            send(j->clientSocket, currentMassage->msg, currentMassage->msgSize, 0);
          }
        }
      }
      delete currentMassage;
    }
    Sleep(10);
  }
}

void massageChannelingMaster() {
  std::thread massageChannelingThread(massageChanneling);
  massageChannelingThread.detach();
//  std::thread massageChanneling2Thread(massageChanneling);
//  massageChanneling2Thread.detach();

  while (true) {
    Sleep(3000);
    if (MsgQueue.size() >= 40000) {
      std::thread massageChannelingHelpThread(massageChanneling);
      massageChannelingHelpThread.detach();
    }
  }

}

void connectionMaster() {
  while (true) {
    Sleep(10);
    if (newConnectionNeeded) {
      std::thread clientInitAndProcessing(Client);
      clientInitAndProcessing.detach();
      connBoolLock.lock();
      newConnectionNeeded = false;
      connBoolLock.unlock();
    }
  }
}


int main() {

#ifdef ALSO_LOG_TO_FILE
  if ((file = fopen(FILEPATH, "w")) == NULL) {
    printf("Cannot open file.\n");
    exit(1);
  }
#endif

  WSAStartup(MAKEWORD(2, 0), &WSAData);
  server = socket(AF_INET, SOCK_STREAM, 0);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(5555);

  bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
//  bool bOptVal = TRUE;
//  int iOptVal = 0;
// std::cout<<setsockopt(server,SOL_SOCKET,SO_REUSEADDR,(char*)&iOptVal,sizeof(int));
  listen(server, 0);

  std::thread massageChannelingMasterThread(massageChannelingMaster);
  massageChannelingMasterThread.detach();
  std::thread connectorMasterThread(connectionMaster);
  connectorMasterThread.join();

  return 0;
}
