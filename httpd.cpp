#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct { char *name, *value; } header_t;
static header_t reqhdr[17] = { 0 };

char *request_header(const char *name)
{
  for (header_t *h = reqhdr; h->name != nullptr; h++)
  {
    if (strcmp(h->name, name) == 0) {
      return h->value;
    }
  }
  return nullptr;
}

void print(int clientfd, const char* msg)
{
  const size_t cmsg = strlen(msg);
  write(clientfd, msg, cmsg);
}

void route(int clientfd, const char* uri)
{
  if (strcmp(uri, "/") == 0) {
    print(clientfd, "HTTP/1.1 200 OK\r\n\r\nHello!");
    return;
  }
  print(clientfd, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
  print(clientfd, "The server has no idea what you want.");
}

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
    char* prot = strtok(nullptr, " \t\r\n");

    fprintf(stderr, "[%s] %s\n", method, uri);

    header_t *h = reqhdr;
    while (h < reqhdr + 16)
    {
      char* k = strtok(nullptr, "\r\n: \t");
      if (k == nullptr) {
        break;
      }

      char* v = strtok(nullptr, "\r\n");
      while (*v && *v == ' ') {
        v++;
      }

      h->name = k;
      h->value = v;
      h++;

      char* t = v + 1 + strlen(v);
      if (t[1] == '\r' && t[2] == '\n') {
        break;
      }
    }

    if (strcmp(method, "GET") == 0) {
      route(clientfd, uri);
    }
  }

  shutdown(clientfd, SHUT_RDWR); // All further send and receive operations are DISABLED...
  close(clientfd);
}

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
  for (struct addrinfo* p = res; p != nullptr; p = p->ai_next)
  {
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    if (listenfd == -1) {
      continue;
    }

    int option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
      break;
    }

    close(listenfd);
  }

  if (listenfd == -1) {
    return -1;
  }

  freeaddrinfo(res);

  if (listen(listenfd, 1000000) != 0) {
    return -1;
  }

  fprintf(stderr, "Server started http://127.0.0.1:%s\n", port);
  return listenfd;
}

int main(int argc, const char* argv[])
{
  int listenfd = startServer("8000");
  if (listenfd == -1) {
    return -1;
  }

  while (1)
  {
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);

    int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);
    if (clientfd >= 0) {
      respond(clientfd);
    }
  }
}
