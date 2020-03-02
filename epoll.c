/**
 * epoll 水平触发模式
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>

int main() {
    int port = 8888;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t serv_len = sizeof(serv_addr);
    socklen_t client_len = sizeof(client_len);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, serv_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    bind(lfd, (struct sockaddr *) &serv_addr, serv_len);
    if (listen(lfd, 36) != -1) {
        printf("listen on %d\n", port);
    }

    //创建epoll数根节点, 指定数的节点为2000
    int epfd = epoll_create(2000);

    // 把 listen fd 放入 epoll 数中
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    struct epoll_event all[2000];

    while (1) {
        /**
         * epoll_wait:
         * epfd: epoll 根节点
         * all: 所有又来接收事件的 epoll_event 数组 ,
         * maxevents: event 的最大数量
         * timeout: 过期时间
         */
        int ret = epoll_wait(epfd, all, 2000, -1);
        for (int i = 0; i < ret; ++i) {
            int fd = all[i].data.fd;
            if (fd == lfd) { //如果是 listen fd
                int cfd = accept(lfd, (struct sockaddr *) &client_addr, &client_len);
                if (cfd == -1) {
                    perror("accept error");
                    exit(1);
                }
                //新的 client fd 挂到树上
                // 把 listen fd 放入 epoll 数中
                struct epoll_event client_ev;
                client_ev.events = EPOLLIN;
                client_ev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &client_ev);
            }else{
                if(! (all[i].events & EPOLLIN) ){ //如果不是读事件，略过
                    continue;
                }
                char buf[1024] = {0};
                int len = recv(fd, buf, sizeof(buf), 0);
                if (len == -1) {
                    perror("recv error");
                    return -1;
                } else if (len == 0) {
                    close(fd);
                    epoll_ctl(epfd,EPOLL_CTL_DEL,fd, NULL);
                } else {
                    printf("recv from client : %s\n", buf);
                    send(fd, buf, strlen(buf) + 1, 0);
                }
            }
        }
    }
}
