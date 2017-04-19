#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <string>
using namespace std;
#define MAXLINE 1024

//This is CLIENT!!

void str_cli(FILE *fp, int sockfd)
{
    int     maxfdp1, stdineof, n;
    fd_set  rset;
    char    sendline[MAXLINE], recvline[MAXLINE];

    stdineof = 0;
    FD_ZERO(&rset);
    for(;;){
        if(stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd)+1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, recvline, MAXLINE)) == 0) {
                if (stdineof != 1)
                    cout << "str_cli: server terminated prematurely" << endl;
                return;
            }
            recvline[n] = 0;
            cout << string(recvline);
        }
        if(FD_ISSET(fileno(fp), &rset))
        {
            if(fgets(sendline, MAXLINE, fp) == NULL)
                cout << "a reading erro occur" << endl;
            if(strcmp(sendline, "exit\n") == 0) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            write(sockfd, sendline, strlen(sendline));
        }
    }
}

int main(int argc, char **argv)
{
    int                 sockfd;
    struct sockaddr_in  servaddr;

    if(argc != 3) {
        cout << "Usage: tcpcli <IPaddr> <Port>" << endl;
        return 1;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t )atoi(argv[2]));

    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        cout << "IP address connect error" << endl;
        return 1;
    }
    if(connect(sockfd,(struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        cout << "connection fail" << endl;
        return 1;
    }
    str_cli(stdin, sockfd);
    return 0;
}
