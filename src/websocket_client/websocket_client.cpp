/********************************************************
 * Description : websocket client
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#include <cstring>
#include <string>
#include "websocket_client.h"
#include "websocket_client_impl.h"
#include "base.h"

WebsocketClientSink::~WebsocketClientSink()
{

}

WebsocketClient::WebsocketClient()
    : m_session(nullptr)
{

}

WebsocketClient::~WebsocketClient()
{
    exit();
}

bool WebsocketClient::init(WebsocketClientSink * sink, const char * host, uint16_t port, bool secure)
{
    std::string url;
    if (nullptr != host && 0x0 != *host && 0 != port)
    {
        url = std::string(secure ? "wss://" : "ws://") + host + ":" + std::to_string(port);
    }
    return init(sink, url.c_str());
}

bool WebsocketClient::init(WebsocketClientSink * sink, const char * url)
{
    exit();

    RUN_LOG_DBG("websocket client init begin");

    if (nullptr == url || 0x0 == *url)
    {
        RUN_LOG_ERR("websocket client init failure while invalid parameters");
        return false;
    }

    if (0 == strncmp(url, "wss", 3))
    {
        m_session = new WebsocketSessionSecure;
    }
    else
    {
        m_session = new WebsocketSessionPlain;
    }

    if (nullptr == m_session)
    {
        RUN_LOG_ERR("websocket client init failure while create websocket session");
        return false;
    }

    if (m_session->init(sink, url))
    {
        RUN_LOG_DBG("websocket client init success");
        return true;
    }

    RUN_LOG_ERR("websocket client init failure");

    exit();

    return false;
}

void WebsocketClient::exit()
{
    if (nullptr != m_session)
    {
        RUN_LOG_DBG("websocket client exit begin");

        m_session->exit();
        delete m_session;
        m_session = nullptr;

        RUN_LOG_DBG("websocket client exit end");
    }
}

void WebsocketClient::connect()
{
    if (nullptr != m_session)
    {
        m_session->connect();
    }
}

void WebsocketClient::close()
{
    if (nullptr != m_session)
    {
        m_session->close();
    }
}

bool WebsocketClient::send_message(const void * data, uint32_t size, bool binary)
{
    return nullptr != m_session && m_session->send_message(data, size, binary);
}

bool WebsocketClient::is_connected() const
{
    return nullptr != m_session && m_session->is_connected();
}
