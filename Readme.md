## 关于此项目

这是一个C/C++编写的、与[go-cqhttp](https://github.com/Mrs4s/go-cqhttp)进行交互实现的QQ机器人

只有Linux amd64版本，没有添加其他平台的计划。

使用的库：

- [libcurl](https://github.com/curl/curl/)
- [libevent](https://github.com/libevent/libevent) 
- [json](https://github.com/nlohmann/json/#projects-using-json-for-modern-c)
- [SQlite3](https://www.sqlite.org)

目前只封装了go-cqhttp的四分之三的API、go-cqhttp支持的常用CQcode，没有对事件进行封装。这意味着制作插件需要自己解析go-cqhttp的事件上报json。