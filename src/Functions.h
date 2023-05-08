#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "MainProcess.h"

void CorePlugin(const string& msg, MainProcess* m);

// libevent的IO回调，当go-cqhttp有HTTP请求的时候调用
void HTTPRequestCB(struct evhttp_request* req, void* arg);

// libevent的事件回调，每隔一段时间调用
void TickEventCB(evutil_socket_t fd, short event, void* arg);

#endif // !FUNCTIONS_H
