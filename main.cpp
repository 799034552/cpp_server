#include"Server.h"
#include<stdio.h>
int main()
{
    setbuf(stdout, NULL);
    Server app;
    app.listen(4399, []{
        printf("server start at 127.0.0.1:4399\n");
    });

    app.get("/", [](Req &req, Res &res){
        res.send("i get :\r\n");
        for(const auto &t: *req.data)
            res.send(t.first + ":" + t.second + "\r\n");

        res.send("hello world");
    });

    app.post("/", [](Req &req, Res &res){
        res.send("i get :\r\n");
        for(const auto &t: *req.data)
            res.send(t.first + ":" + t.second + "\r\n");
        res.send("hello world");
    });

    app.get("/main", [](Req &req, Res &res){
        res.send("hello world this is main");
    });
    app.run();
}