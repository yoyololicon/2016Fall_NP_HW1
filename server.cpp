#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <string>
using namespace std;
#define MAXLINE 1024

string Hello = "[Server] Hello, anonymous! From: ";
string wellcome = "[Server] Someone is coming!\n";
string Serv = "[Server] ";
string off = " is offline.\n";

string cantBeAnonymous = "[Server] ERROR: Username cannot be anonymous.\n";
string hasBeenUsed_1 = "[Server] ERROR: ";
string hasBeenUsed_2 = " has been used by others.\n";
string name_unconsist = "[Server] ERROR: Username can only consists of 2~12 English letters.\n";
string new_name_1 = "[Server] You're now known as ";
string new_name_2 = " is now known as ";

string UR_anonymous = "[Server] ERROR: You are anonymous.\n";
string U_sent_anonymous = "[Server] ERROR: The client to which you sent is anonymous.\n";
string no_exist = "[Server] ERROR: The receiver doesn't exist.\n";
string sent_success = "[Server] SUCCESS: Your message has been sent.\n";

string err = "[Server] ERROR: Error command.\n";

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;
    int white = 0;

    ptr = (char*)vptr;
    for(n = 1; n < maxlen; n++)
    {
        again:
        if((rc = read(fd, &c, 1)) == 1) {
            if (white) {
                if (c != ' ') {
                    white = 0;
                    *ptr++ = c;
                }
            } else {
                if (c == ' ')
                    white = 1;
                *ptr++ = c;
            }
            if (c == '\n')
                break;
        }
        else if(rc == 0)
        {
            *ptr = 0;
            return n-1;
        }
        else
        {
            if(errno == EINTR)
                goto again;
            return -1;
        }
    }

    *ptr = 0;
    return n;
}

char *sock_ntop(const struct sockaddr_in *sa, socklen_t salen) {
    char portstr[8];
    static char str[128];        /* Unix domain is largest */

    if (inet_ntop(AF_INET, &sa->sin_addr, str, sizeof(str)) == NULL)
        return (NULL);
    if (ntohs(sa->sin_port) != 0) {
        snprintf(portstr, sizeof(portstr), "/%d", ntohs(sa->sin_port));
        strcat(str, portstr);
    }
    return (str);
}

int message(int fd, string &mes)
{
    char tmp[MAXLINE];
    strcpy(tmp, mes.c_str());
    if(write(fd, tmp, strlen(tmp)) < 0) {
        cout << "fail to sent message" << endl;
        return 0;
    }
    return 1;
}

void broadcast(int *client, int except, int maxi, string &mes)
{
    for(int i = 0; i <= maxi; i++)
    {
        if(i != except && client[i] >= 0)
            message(client[i], mes);
    }
}

int english_test(char* c, int len)
{
    for(int i = 0; i < len; i++)
    {
        if(c[i] > 'z')
            return 1;
        else if(c[i] < 'a' && c[i] > 'Z')
            return 1;
        else if(c[i] < 'A')
            return 1;
    }
    return 0;
}

int name_test(char* c, string *name, int from, int *client, int maxi)
{
    string tmp(c);
    int len = tmp.size();
    if(tmp == "anonymous")
        return 1;
    else if(len < 2 || len > 12)
        return 2;
    else if(english_test(c, len))
        return 2;
    else
    {
        for(int i = 0; i <= maxi; i++)
        {
            if(name[i] == tmp)
                return 3;
        }
        string old_name = name[from];
        name[from] = tmp;
        tmp = new_name_1 + name[from] + "\n";
        message(client[from], tmp);
        tmp = Serv + old_name + new_name_2 + name[from] + "\n";
        broadcast(client, from, maxi, tmp);
        return 0;
    }
}

void send_message(string &mes, string &to, int *client, int from, string *name, int maxi)
{
    if(name[from] == "anonymous")
        message(client[from], UR_anonymous);
    else if(to == "anonymous")
        message(client[from], U_sent_anonymous);
    else
    {
        for(int i = 0; i <= maxi; i++)
        {
            if(name[i] == to)
            {
                string tmp = Serv + name[from] + " tell you " + mes;
                if(message(client[i], tmp))
                    message(client[from], sent_success);
                return;
            }
        }
        message(client[from], no_exist);
        return;
    }
}

int
main(int argc, char **argv)
{
    int					i, maxi, maxfd, listenfd, connfd, sockfd;
    int					nready, client[FD_SETSIZE];
    pid_t               pid;
    ssize_t				n;
    fd_set				rset, allset;
    char				buf[MAXLINE];
    socklen_t			clilen;
    struct sockaddr_in	cliaddr, servaddr;
    string              tmp, old_name;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(20000);

    if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        cout << "binding error" << endl;
        return 1;
    }
    listen(listenfd, 64);

    maxfd = listenfd;			/* initialize */
    maxi = -1;					/* index into client[] array */
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;			/* -1 indicates available entry */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    for ( ; ; ) {
        rset = allset;		/* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {	/* new client connection */

            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
            if((pid = fork()) == 0)
            {
                close(listenfd);
                string addr(sock_ntop(&cliaddr, clilen));
                tmp = Hello + addr + "\n";
                string name = "anonymous";


            }
            else
                close(connfd);


            if (i == FD_SETSIZE)
                cout << "too many clients" << endl;

            FD_SET(connfd, &allset);	/* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;			/* for select */
            if (i > maxi)
                maxi = i;				/* max index in client[] array */
            message(client[i], tmp);
            broadcast(client, i, maxi, wellcome);
            if (--nready <= 0)
                continue;				/* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++) {	/* check all clients for data */
            if ( (sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = readline(sockfd, buf, MAXLINE)) == 0) {
                    /*4connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    tmp = Serv + name[i] + off;
                    broadcast(client, -1, maxi, tmp);
                }
                else if(strncmp(buf, "who\n", 4) == 0)
                {
                    for(int j = 0; j <= maxi; j++)
                    {
                        if(client[j] >= 0)
                        {
                            tmp = Serv + name[j] + " " + addr[j];
                            if(i == j)
                                tmp+=" ->me";
                            tmp+="\n";
                            message(client[i], tmp);
                        }
                    }
                }
                else if(strncmp(buf, "name ", 5) == 0)
                {
                    buf[strlen(buf)-1] = 0;
                    n = name_test(buf+5,  name, i, client, maxi);
                    switch (n)
                    {
                        case 1:
                            message(client[i], cantBeAnonymous);
                            break;
                        case 2:
                            message(client[i], name_unconsist);
                            break;
                        case 3:
                            tmp = hasBeenUsed_1+ string(buf+5) + hasBeenUsed_2;
                            message(client[i], tmp);
                            break;
                        default:
                            break;
                    }
                }
                else if(strncmp(buf, "tell ", 5) == 0)
                {
                    int len = strlen(buf+5);
                    char* ptr = strtok(buf+5, " ");
                    string to(ptr);
                    if(len != to.size()) {
                        ptr = ptr + to.size() + 1;
                        string mes(ptr);
                        send_message(mes, to, client, i, name, maxi);
                    }
                    else
                        message(client[i], err);
                }
                else if(strncmp(buf, "yell ", 5) == 0)
                {
                    tmp = Serv + name[i] + " yell " + string(buf+5);
                    broadcast(client, i, maxi, tmp);
                }
                else
                    message(client[i], err);

                if (--nready <= 0)
                    break;				/* no more readable descriptors */
            }
        }
    }
#pragma clang diagnostic pop
}