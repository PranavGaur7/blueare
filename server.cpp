#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>  
#include <cstring>

static void die(const char* msg) {
    perror(msg);
    exit(1);
}

static void do_something(int connfd) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        die("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    ssize_t bytes_written = write(connfd, wbuf, strlen(wbuf));
    if (bytes_written < 0) {
        die("write() error");
    }
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int val = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);        
    addr.sin_addr.s_addr = htonl(0);    

    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) { die("bind()"); }


    rv = listen(fd, SOMAXCONN);
    if (rv) { die("listen()"); }

    while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;   // error
        }

        do_something(connfd);
        close(connfd);
    }

    return 0;
}