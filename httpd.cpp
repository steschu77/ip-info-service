#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>

#include <time.h>

// ----------------------------------------------------------------------------
long long getUTCTimeInMS()
{
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}

// ----------------------------------------------------------------------------
void appHandleGetReq(int clientfd, const char* uri)
{
  FILE* f = fdopen(clientfd, "w");
  
  if (strcmp(uri, "/") == 0) {
    fprintf(f, "HTTP/1.1 200 OK\r\n\r\nHello!");
  } else if (strcmp(uri, "/time") == 0) {
    fprintf(f, "HTTP/1.1 200 OK\r\n\r\n");
    fprintf(f, "%lld", getUTCTimeInMS());
  } else {
    fprintf(f, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
  }
  fclose(f);
}

// ----------------------------------------------------------------------------
void respond(int clientfd)
{
  char buf[65535];
  int rcvd = recv(clientfd, buf, 65535, 0);

  if (rcvd <= 0) {
    fprintf(stderr, rcvd < 0 ? "recv() error\n" : "Client disconnected unexpectedly.\n");
  }
  else // message received
  {
    buf[rcvd] = '\0';

    char* method = strtok(buf, " \t\r\n"); // "GET" or "POST"
    char* uri = strtok(nullptr, " \t");    // "/index.html" incl. '?'

    fprintf(stderr, "[%s] %s\n", method, uri);

    if (strcmp(method, "GET") == 0) {
      appHandleGetReq(clientfd, uri);
    }
  }

  shutdown(clientfd, SHUT_RDWR); // All further send and receive operations are DISABLED...
  close(clientfd);
}

// ----------------------------------------------------------------------------
int startServer(const char *port)
{
  struct addrinfo hints = { 0 }, *res = nullptr;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(nullptr, port, &hints, &res) != 0) {
    return -1;
  }

  int listenfd = -1;
  int ipAddr = -1;
  for (struct addrinfo* p = res; p != nullptr; p = p->ai_next)
  {
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    if (listenfd == -1) {
      continue;
    }

    int option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
      ipAddr = reinterpret_cast<const sockaddr_in*>(p->ai_addr)->sin_addr.s_addr;
      break;
    }

    close(listenfd);
  }

  freeaddrinfo(res);

  if (listenfd == -1) {
    return -1;
  }

  if (listen(listenfd, 1000000) != 0) {
    return -1;
  }

  fprintf(stderr, "Server started http://%d.%d.%d.%d:%s\n",
    ipAddr&0xff, (ipAddr>>8)&0xff, (ipAddr>>16)&0xff, (ipAddr>>24)&0xff, port);
  return listenfd;
}

// ----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  int listenfd = startServer("8000");
  if (listenfd == -1) {
    return -1;
  }

  while (1)
  {
    struct sockaddr_in peerAddr;
    socklen_t peerAddrLen = sizeof(peerAddr);

    int clientfd = accept(listenfd, (struct sockaddr *)&peerAddr, &peerAddrLen);
    if (clientfd >= 0) {
      int ipAddr = peerAddr.sin_addr.s_addr;
      fprintf(stderr, "Client connect %d.%d.%d.%d:%d\n",
        ipAddr&0xff, (ipAddr>>8)&0xff, (ipAddr>>16)&0xff, (ipAddr>>24)&0xff,
        peerAddr.sin_port);
      respond(clientfd);
    }
  }
}
