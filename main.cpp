#include"Server.h"
#include<stdio.h>
int main()
{
    setbuf(stdout, NULL);
    Server app;
    app.listen(4399, []{
        printf("server start at 127.0.0.1:4399\n");
    },1);

    app.get("/", [](Req &req, Res &res){
        res.send("i get :\r\n");
        for(const auto &t: *req.data)
            res.send(t.first + ":" + t.second + "\r\n");

        res.send("hello world");
    });

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
        //res.send("hello world");
    });
    vector<std::shared_ptr<WSClient>> socket_list;
    string b = "@3432";
    app.ws("/websocket", [&](std::shared_ptr<WSPool> io){
        io->on_connect([&](std::shared_ptr<WSClient> socket){
            socket_list.push_back(socket);
            socket->on_get([&](string text, std::shared_ptr<WSClient> sp_socket) {
                sp_socket->emit(text);
                //socket->emit("456");
            });
            // socket_list.push_back(socket);
            socket->emit("hello");
            //printf("new connect----------------\n");
        });
    });

    app.run();
}

// #include"Server.h"
// #include "rapidjson/document.h"
// #include "rapidjson/writer.h"
// #include "rapidjson/stringbuffer.h"
// #include <iostream>
// using namespace std;

// using namespace rapidjson;

// int main() {
//     //解析json
//     const string json("{asdfasdf\":234}");
//     Document d;
//     d.Parse(json.c_str());
//     if (d.IsNull())
//         cout<<"no json";
//     else
//         cout<<"json";
//     cout<<d.IsNull();
//     return 0;
//     // 获取值
//     cout<<d["stars"].GetInt()<<endl;
//     // 遍历
//     for(auto a = d.MemberBegin(); a != d.MemberEnd(); ++a)
//         cout<<a->name.GetString()<<"  \n";
//     // 遍历数组
//     const Value& c = d["scores"];
//     for(rapidjson::SizeType i = 0; i < c.Size(); ++i)
//     {
//         cout<<c[i].GetInt()<<"  ";
//     }

//     //添加json
//     Document myjson;
//     myjson.SetObject();

//     Value m_array;
//     m_array.SetArray();
//     m_array.PushBack(1, myjson.GetAllocator());
//     m_array.PushBack("shabi", myjson.GetAllocator());

//     myjson.AddMember("name","haha",myjson.GetAllocator());
//     myjson.AddMember("id","123",myjson.GetAllocator());
//     myjson.AddMember("array" ,m_array, myjson.GetAllocator());

//     for(auto a = myjson.MemberBegin(); a != myjson.MemberEnd(); ++a)
//     cout<<a->name.GetString()<<"  \n";
// }