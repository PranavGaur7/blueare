#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ip.h>

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    return 0;
}