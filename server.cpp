#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>

using namespace std;

struct pollfd
{
    int fd;
    short events;  // request: want to read, write, or both?
    short revents; // returned: can read? can write?
};

struct Conn
{
    int fd = -1;
    // application's intention, for the event loop
    bool want_read = false;
    bool want_write = false;
    bool want_close = false;
    // buffered input and output
    vector<uint8_t> incoming; // data to be parsed by the application
    vector<uint8_t> outgoing; // responses generated by the application
};

static void msg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
        {
            return -1; // error, or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0)
        {
            return -1; // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t one_request(int connfd)
{
    // 4 bytes header
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err)
    {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4); // assume little endian
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    // request body
    err = read_full(connfd, &rbuf[4], len);
    if (err)
    {
        msg("read() error");
        return err;
    }

    // do something
    fprintf(stderr, "client says: %.*s\n", len, &rbuf[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd, wbuf, 4 + len);
}

static void fd_set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0); // get the flags
    flags |= O_NONBLOCK;               // modify the flags
    fcntl(fd, F_SETFL, flags);         // set the flags
    // TODO: handle errno
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    // this is needed for most server applications
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        die("bind()");
    }

    // listen
    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        die("listen()");
    }
    // a map of all client connections, keyed by fd
    vector<Conn *> fd2conn;
    //  event loop
    vector<struct pollfd> poll_args;
    while (true)
    {
        //  prepare the arguments of the polls
        poll_args.clear();
        // put the listening sockets in the first position
        struct pollfd pfd = {fd, POLLIN, 0};
        // the rest are connection sockets
        poll_args.push_back(pfd);
        for (Conn *conn : fd2conn)
        {
            if (!conn)
                continue;
            struct pollfd pfd = {conn->fd, POLLERR, 0};
            // poll() flags from the application's intent
            if (conn->want_read)
                pfd.events |= POLLIN;
            if (conn->want_write)
                pfd.events |= POLLOUT;
            poll_args.push_back(pfd);
        }

        // wait for readiness
        int rv = poll(poll_args.data(),(nfds_t)poll_args.size(),-1);
        if(rv<0 && errno==EINTR){
            continue; // not an error
        }
        if(rv<0){
            die("poll");
        }

        // handle the listening socket

        if(poll_args[0].revents){
            if(Conn *conn = handle_accept(fd)){
                // Ensure vector size can accommodate new fd
                if(fd2conn.size()<= (size_t)conn->fd){
                    fd2conn.resize(conn->fd+1);
                }
                fd2conn[conn->fd] = conn;
            }
        }
        // handle connection sockets
        for(size_t i=1;i<poll_args.size();i++){ // note: skip the 1st
            uint32_t ready = poll_args[i].revents;
            Conn *conn = fd2conn[poll_args[i].fd];
            if(ready & POLLIN) handle_read(conn);
            if(ready & POLLOUT) handle_write(conn);
            if((ready & POLLERR) || conn->want_close){
                (void) close(conn->fd);
                fd2conn[conn->fd] = NULL;
                delete conn;
            }
        }
    }
    return 0;
}