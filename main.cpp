#include"Server.h"
#include<stdio.h>
int main()
{
    //成为守护进程
    //daemon(1, 0);
    Server app;
    app.listen(4399, []{
        printf("server start at 127.0.0.1:4399\n");
    },3);

    // app.get("/", [](Req &req, Res &res){
    //     res.send("hello");
    //     // res.send("i get :\r\n");
    //     // for(const auto &t: *req.data)
    //     //     res.send(t.first + ":" + t.second + "\r\n");
    //     // res.send("hello world");
    // });

    app.post("/", [](Req &req, Res &res){
        if (req.is_json)
        {
            res.add_header("Content-Type", "application/json");
            res.send("{");
            for(const auto &t: *req.data)
                res.send("\"" + t.first +  "\":" + "\"" +t.second + "\",");
            res.send("}");
        }
        else
        {
            res.send(*req.text);
        }
    });
    app.ws("/websocket", [](SP_WSPool io){
        io->on_connect([](SP_WSClient socket){
            socket->on_get([](string text, SP_WSClient _this) {
                _this->emit(text);
            });
            socket->emit("hello");
        });
    });

    app.run();
}
