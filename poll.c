#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <netinet/in.h>

int main() {
    int port = 8888;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t serv_len = sizeof(serv_addr);
    socklen_t client_len = sizeof(client_len);

    int lfd = socket(AF_INET,SOCK_STREAM,0);
    memset(&serv_addr,0,serv_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    bind(lfd,(struct sockaddr*)&serv_addr , serv_len);
    if( listen(lfd,36) != -1 ){
        printf("listen on %d\n", port);
    }

    //pollfd 初始化
    struct pollfd allfd[1024];
    int max_index = 0;
    int i = 0 ;

    for (i = 0; i < 1024; ++i) {
        allfd[i].fd = -1;
        allfd[i].events = POLLIN;
    }
    allfd[0].fd = lfd;

    while (1){
        /**
         * poll 函数参数：
         * 1：pollfd,内核要检测的文件描述符数组
         * 2: nfds, 要检测的文件描述符个数；
         * 3，timeout, -1 永久阻塞直到有时间发生；0，立即返回；其他正数，表示毫秒数；
         */
        int ret = poll(allfd, max_index + 1 ,-1);
        if( ret == -1  ){
            perror("poll error");
            return -1;
        }
        if( allfd[0].revents & POLLIN ){  // 有了新的连接请求，accept() ，此处不会阻塞；
            int cfd = accept(lfd,(struct sockaddr *)&client_addr,&client_len);
            if( cfd == -1 ){
                perror("accept error");
                exit(1);
            }
            for (i = 1; i < 1024; ++i) {
                if( allfd[i].fd == -1 ){
                    allfd[i].fd = cfd;
                    break;
                }
            }
            max_index = max_index < i ? i : max_index;
        }

        for (i = 1; i <= max_index; ++i) {
            if( allfd[i].fd == -1 ){
                continue;
            }
            if( allfd[i].revents & POLLIN ){
                char buf[1024] = {0};
                int len = recv(allfd[i].fd,buf, sizeof(buf),0);
                if( len == -1 ){
                    perror("recv error");
                    return -1;
                }else if(len == 0 ){
                    allfd[i].fd = -1;
                    close(allfd[i].fd);
                }else{
                    printf("recv from client : %s\n", buf);
                    send(allfd[i].fd,buf,strlen(buf) + 1,0);
                }
            }
        }
    }
}
