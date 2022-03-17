# expressjs风格的c++ 服务器
> linux c++服务器，由于c++语法本身繁琐，大多数的cpp服务器都不能像node服务器一样能快速上手，因此，打算做一个仿`expressjs`风格的c++服务器，懂一点点c++就能快速上手部署。

做这个项目给最近学的知识作一个总结贯通吧。
## 项目特点
* expressjs风格，用起来语法简单
* 支持get/post
* 半同步/半异步并发模型（Reactor模型）,ET边缘触发，eventfd唤醒子进程
* C++11特性，bind，shared_ptr,std::function应用

## 快速上手
```c
#include"Server.h"
#include<stdio.h>
int main()
{
    Server app;
    app.get("/", [](Req &req, Res &res){
        res.send("hello world");
    });
    app.listen(8080);
    app.run();
}
```
够简单了吧，不用看文档都知道是什么意思了
## 安装
```shell
mkdir build && cd build && cmake .. && make
```
# todo
- [ ] 添加json支持
- [ ] 添加websocket支持
- [ ] 添加日志系统

