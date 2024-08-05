/********************************************************
 * Description : enet client
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#ifndef ENET_CLIENT_H
#define ENET_CLIENT_H


#include <cstdint>
#include "base.h"

struct GOOFER_API EnetClientSink
{
    virtual ~EnetClientSink();
    virtual void on_enet_connect() = 0;
    virtual void on_enet_close() = 0;
    virtual void on_enet_error(const char * action, const char * message) = 0;
    virtual void on_enet_recv(const void * data, uint32_t size) = 0;
};

class EnetClientImpl;

class GOOFER_API EnetClient
{
public:
    EnetClient();
    ~EnetClient();

public:
    bool init(EnetClientSink * sink, const char * host, uint16_t port);
    void exit();

public:
    void connect();
    void close();
    bool send_message(const void * data, uint32_t size);
    bool is_connected() const;

private:
    EnetClient(const EnetClient &) = delete;
    EnetClient(EnetClient &&) = delete;
    EnetClient & operator = (const EnetClient &) = delete;
    EnetClient & operator = (EnetClient &&) = delete;

private:
    EnetClientImpl                * m_impl;
};


#endif // ENET_CLIENT_H
