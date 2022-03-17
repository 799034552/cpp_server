#include"Server.h"
#include<sys/epoll.h>
#include<stdio.h>
int main()
{
    setbuf(stdout, NULL);
    Server app;
    app.listen(4399, []{
        printf("server start at 127.0.0.1:4399\n");
    },3);
    app.get("/", [](Req &req, Res &res){
        res.send("hello world");
    });
    app.run();


}