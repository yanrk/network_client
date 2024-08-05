/********************************************************
 * Description : enet client
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#include "enet_client.h"
#include "enet_client_impl.h"

EnetClientSink::~EnetClientSink()
{

}

EnetClient::EnetClient()
    : m_impl(nullptr)
{

}

EnetClient::~EnetClient()
{
    exit();
}

bool EnetClient::init(EnetClientSink * sink, const char * host, uint16_t port)
{
    exit();

    do
    {
        m_impl = new EnetClientImpl;
        if (nullptr == m_impl)
        {
            break;
        }

        if (!m_impl->init(sink, host, port))
        {
            break;
        }

        return true;
    } while (false);

    exit();

    return false;
}

void EnetClient::exit()
{
    if (nullptr != m_impl)
    {
        m_impl->exit();
        delete m_impl;
        m_impl = nullptr;
    }
}

void EnetClient::connect()
{
    if (nullptr != m_impl)
    {
        m_impl->connect();
    }
}

void EnetClient::close()
{
    if (nullptr != m_impl)
    {
        m_impl->close();
    }
}

bool EnetClient::send_message(const void * data, uint32_t size)
{
    return nullptr != m_impl && m_impl->send_message(data, size);
}

bool EnetClient::is_connected() const
{
    return nullptr != m_impl && m_impl->is_connected();
}
