/********************************************************
 * Description : websocket client
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H


#include <cstdint>
#include "base.h"

struct GOOFER_API WebsocketClientSink
{
    virtual ~WebsocketClientSink();
    virtual void on_websocket_connect() = 0;
    virtual void on_websocket_close() = 0;
    virtual void on_websocket_error(const char * action, const char * message) = 0;
    virtual void on_websocket_recv(const void * data, uint32_t size, bool binary) = 0;
};

class WebsocketSessionBase;

class GOOFER_API WebsocketClient
{
public:
    WebsocketClient();
    ~WebsocketClient();

public:
    bool init(WebsocketClientSink * sink, const char * host, uint16_t port, bool secure);
    bool init(WebsocketClientSink * sink, const char * url);
    void exit();

public:
    void connect();
    void close();
    bool send_message(const void * data, uint32_t size, bool binary);
    bool is_connected() const;

private:
    WebsocketClient(const WebsocketClient &) = delete;
    WebsocketClient(WebsocketClient &&) = delete;
    WebsocketClient & operator = (const WebsocketClient &) = delete;
    WebsocketClient & operator = (WebsocketClient &&) = delete;

private:
    WebsocketSessionBase          * m_session;
};


#endif // WEBSOCKET_CLIENT_H
