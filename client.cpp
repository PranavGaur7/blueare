#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int fd,char* buf,size_t n){
    while(n>0){
        ssize_t rv = read(fd,buf,n);
        if(rv<=0) return -1;
        assert((size_t)rv<=n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}
static int32_t write_full(int fd,const char* buf,size_t n){
    while(n>0){
        ssize_t rv = write(fd,buf,n);
        if(rv<=0) return -1;
        assert((size_t)rv<=n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        die("connect");
    }

    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        die("read");
    }
    printf("server says: %s\n", rbuf);
    close(fd);
    return 0;
}