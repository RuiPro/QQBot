## 关于此项目

这是一个C/C++编写的、与[go-cqhttp](https://github.com/Mrs4s/go-cqhttp)进行交互实现的QQ机器人

> 2023.10.11 go-cqhttp开发团队因tx的堵截，已无力维护项目，但目前仍可继续使用[qsign](https://github.com/fuqiuluo/unidbg-fetch-qsign)维持登录，在不久的将来这种方式也将失效。
>
> 新的解决方案已经出现，此项目也将等待新的解决方案稳定进而迁移。

只有Linux amd64版本，没有添加其他平台的计划。

使用的库：

- [libcurl](https://github.com/curl/curl/)
- [libevent](https://github.com/libevent/libevent) 
- [json](https://github.com/nlohmann/json/#projects-using-json-for-modern-c)
- [SQlite3](https://www.sqlite.org)

目前只封装了go-cqhttp的90%的API、go-cqhttp支持的常用CQcode，没有对事件进行封装。这意味着制作插件需要自己解析go-cqhttp的事件上报json。

有关插件的编译: [QQBotPlugin](https://github.com/RuiPro/QQBotPlugin)