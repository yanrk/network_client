/********************************************************
 * Description : sample of enet & websocket clients
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include "base.h"
#include "enet_client.h"
#include "websocket_client.h"

class NetworkClientTester : public WebsocketClientSink, public EnetClientSink
{
public:
    NetworkClientTester();
    virtual ~NetworkClientTester();

public:
    bool init(const std::string & peer_host, uint16_t peer_port, bool use_udp);
    void exit();

public:
    void connect();
    void close();
    bool send_message(const void * data, uint32_t size);
    bool is_connected() const;

public:
    void on_connect(const char * operater);
    void on_close(const char * operater);
    void on_error(const char * operater, const char * action, const char * message);
    void on_recv(const char * operater, const void * data, uint32_t size);

public:
    virtual void on_websocket_connect() override;
    virtual void on_websocket_close() override;
    virtual void on_websocket_error(const char * action, const char * message) override;
    virtual void on_websocket_recv(const void * data, uint32_t size, bool binary) override;

public:
    virtual void on_enet_connect() override;
    virtual void on_enet_close() override;
    virtual void on_enet_error(const char * action, const char * message) override;
    virtual void on_enet_recv(const void * data, uint32_t size) override;

private:
    bool handle_message(const void * data, uint32_t size);

private:
    bool                                                    m_running;
    bool                                                    m_use_udp;
    std::string                                             m_peer_host;
    uint16_t                                                m_peer_port;
    WebsocketClient                                         m_web_client;
    EnetClient                                              m_udp_client;
};

NetworkClientTester::NetworkClientTester()
    : m_running(false)
    , m_use_udp(false)
    , m_peer_host()
    , m_peer_port(0)
    , m_web_client()
    , m_udp_client()
{

}

NetworkClientTester::~NetworkClientTester()
{
    exit();
}

bool NetworkClientTester::init(const std::string & peer_host, uint16_t peer_port, bool use_udp)
{
    exit();

    do
    {
        RUN_LOG_DBG("network client tester init begin");

        m_running = true;

        m_use_udp = use_udp;
        m_peer_host = peer_host;
        m_peer_port = peer_port;

        if (m_use_udp)
        {
            if (!m_udp_client.init(this, m_peer_host.c_str(), m_peer_port))
            {
                RUN_LOG_ERR("network client tester init failure while init enet client failed");
                break;
            }
        }
        else
        {
            if (!m_web_client.init(this, m_peer_host.c_str(), m_peer_port, false))
            {
                RUN_LOG_ERR("network client tester init failure while init websocket client failed");
                break;
            }
        }

        RUN_LOG_DBG("network client tester try connect");

        connect();

        RUN_LOG_DBG("network client tester init success");

        return true;
    } while (false);

    RUN_LOG_ERR("network client tester init failure");

    exit();

    return false;
}

void NetworkClientTester::exit()
{
    if (m_running)
    {
        RUN_LOG_DBG("network client tester exit begin");

        m_running = false;

        m_web_client.exit();
        m_udp_client.exit();

        RUN_LOG_DBG("network client tester exit end");
    }
}

void NetworkClientTester::connect()
{
    if (!m_running)
    {
        return;
    }

    if (m_use_udp)
    {
        m_udp_client.connect();
    }
    else
    {
        m_web_client.connect();
    }
}

void NetworkClientTester::close()
{
    if (m_use_udp)
    {
        m_udp_client.close();
    }
    else
    {
        m_web_client.close();
    }
}

bool NetworkClientTester::send_message(const void * data, uint32_t size)
{
    if (m_use_udp)
    {
        return m_udp_client.send_message(data, size);
    }
    else
    {
        return m_web_client.send_message(data, size, true);
    }
}

bool NetworkClientTester::is_connected() const
{
    if (m_use_udp)
    {
        return m_udp_client.is_connected();
    }
    else
    {
        return m_web_client.is_connected();
    }
}

void NetworkClientTester::on_connect(const char * operater)
{
    RUN_LOG_DBG("network client tester connected by %s", operater);
}

void NetworkClientTester::on_close(const char * operater)
{
    RUN_LOG_DBG("network client tester disconnected on %s", operater);
    connect();
    RUN_LOG_DBG("network client tester try reconnect by %s", operater);
}

void NetworkClientTester::on_error(const char * operater, const char * action, const char * message)
{
    RUN_LOG_DBG("network client tester network notify, %s %s error: %s", operater, action, message);
}

void NetworkClientTester::on_recv(const char * operater, const void * data, uint32_t size)
{
    if (nullptr == data || 0 == size)
    {
        RUN_LOG_ERR("%s recv data failed while data size (%u)", operater, size);
        close();
        return;
    }

    if (!handle_message(data, size))
    {
        RUN_LOG_ERR("%s recv data failed while handle data message failure", operater);
        close();
        return;
    }
}

void NetworkClientTester::on_websocket_connect()
{
    on_connect("websocket client");
}

void NetworkClientTester::on_websocket_close()
{
    on_close("websocket client");
}

void NetworkClientTester::on_websocket_error(const char * action, const char * message)
{
    on_error("websocket client", action, message);
}

void NetworkClientTester::on_websocket_recv(const void * data, uint32_t size, bool binary)
{
    on_recv("websocket client", data, size);
}

void NetworkClientTester::on_enet_connect()
{
    on_connect("enet client");
}

void NetworkClientTester::on_enet_close()
{
    on_close("enet client");
}

void NetworkClientTester::on_enet_error(const char * action, const char * message)
{
    on_error("enet client", action, message);
}

void NetworkClientTester::on_enet_recv(const void * data, uint32_t size)
{
    on_recv("enet client", data, size);
}

bool NetworkClientTester::handle_message(const void * data, uint32_t size)
{
    RUN_LOG_DBG("handle message size (%u)", size);
    return true;
}

int main(int argc, char * argv[])
{
    printf("usage: %s host port udp\n", argv[0]);

    const char * host = (argc > 1 ? argv[1] : "172.16.4.16");
    uint16_t port = static_cast<uint16_t>(argc > 2 ? atoi(argv[2]) : 20001);
    bool use_udp = (argc > 3 ? 0 != atoi(argv[3]) : false);

    NetworkClientTester tester;
    if (!tester.init(host, port, use_udp))
    {
        return 1;
    }

    while (true)
    {
        getchar();
    }

    tester.exit();

    return 0;
}
