#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
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

    int nfds = lfd;

    fd_set reads , temp; // reads 存的原始数据； temp临时的

    FD_ZERO(&reads); //把读集合标志位都置为0
    FD_SET(lfd,&reads); //把监听描述符加入读集合中
    while (1){

        temp = reads; //因为内核要修改读文件描述符集合，所以原始集合不能传给 select, 传递一个 temp
        /**
         * select 函数参数：
         * 1：nfds,内核要检测的最大文件描述符
         * 2：要检测读操作的文件描述符
         * 3: 写操作的文件描述符
         * 4: 异常的文件描述符
         * 5: timeval类型，null, 永久阻塞，直到有事件发生；否则就阻塞到设定的时间。
         */
        int ret = select( nfds + 1 , &temp , NULL , NULL , NULL); //写集合，异常集合 都置为NULL，timeout 也置为null
        if( ret == -1  ){
           perror("select error");
            return -1;
        }
        if( FD_ISSET(lfd,&temp) ){ // 有了新的连接请求，accept() ，此处不会阻塞；
            int cfd = accept(lfd,(struct sockaddr *)&client_addr,&client_len);
            if( cfd == -1 ){
                perror("accept error");
                exit(1);
            }
            printf("new client connected \n");
            // 将 客户端 加入到待检测的 读集合
            FD_SET(cfd,&reads);
            nfds = nfds < cfd ? cfd : nfds;
        }
        for (int i = lfd + 1; i <= nfds; ++i) {
            if( FD_ISSET(i,&temp) ){
                char buf[1024] = {0};
                int len = recv(i,buf, sizeof(buf),0);
                if( len == -1 ){
                    perror("recv error");
                    return -1;
                }else if(len == 0 ){
                    close(i);
                    FD_CLR(i,&reads);
                }else{
                    printf("recv from client : %s\n", buf);
                    send(i,buf,strlen(buf) + 1,0);
                }
            }
        }
    }
}
