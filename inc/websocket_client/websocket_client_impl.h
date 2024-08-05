/********************************************************
 * Description : websocket client implement
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#ifndef WEBSOCKET_CLIENT_IMPL_H
#define WEBSOCKET_CLIENT_IMPL_H


#include <list>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#define ASIO_STANDALONE
#define _WEBSOCKETPP_NULLPTR_
#define _WEBSOCKETPP_INITIALIZER_LISTS_
#define _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_FUNCTIONAL_
#define _WEBSOCKETPP_CPP11_MEMORY_
#define _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_

#include "websocketpp/config/asio_client.hpp"
#include "websocketpp/client.hpp"

#include "websocket_client.h"
#include "base.h"

class WebsocketSessionBase
{
public:
    WebsocketSessionBase();
    virtual ~WebsocketSessionBase();

public:
    virtual bool init(WebsocketClientSink * sink, const std::string & url) = 0;
    virtual void exit() = 0;

public:
    virtual void connect() = 0;
    virtual void close() = 0;
    virtual bool send_message(const void * data, uint32_t size, bool binary) = 0;
    virtual bool is_connected() const = 0;
};

template <typename client_type>
class WebsocketSession : public WebsocketSessionBase
{
public:
    WebsocketSession();
    virtual ~WebsocketSession();

public:
    virtual bool init(WebsocketClientSink * sink, const std::string & url) override;
    virtual void exit() override;

public:
    virtual void connect() override;
    virtual void close() override;
    virtual bool send_message(const void * data, uint32_t size, bool binary) override;
    virtual bool is_connected() const override;

protected:
    websocketpp::client<client_type> & get_client();
    void set_handle(websocketpp::connection_hdl handle);
    void on_error(const char * action, const char * message);

private:
    void on_connect();
    void on_close();

private:
    void do_connect();
    void do_close();

private:
    void real_connect();
    void real_close();

private:
    virtual void set_specific_handler() = 0;

private:
    bool                                                    m_running;
    bool                                                    m_working;
    WebsocketClientSink                                   * m_sink;
    std::string                                             m_url;
    websocketpp::client<client_type>                        m_client;
    websocketpp::connection_hdl                             m_handle;
    std::thread                                             m_work_thread;

private:
    std::list<bool>                                         m_event_list;
    std::mutex                                              m_event_mutex;
    std::condition_variable                                 m_event_condition;
    std::thread                                             m_event_thread;

private:
    std::list<bool>                                         m_callback_list;
    std::mutex                                              m_callback_mutex;
    std::condition_variable                                 m_callback_condition;
    std::thread                                             m_callback_thread;
};

template <typename client_type>
WebsocketSession<client_type>::WebsocketSession()
    : m_running(false)
    , m_working(false)
    , m_sink(nullptr)
    , m_url()
    , m_client()
    , m_handle()
    , m_work_thread()
    , m_event_list()
    , m_event_mutex()
    , m_event_condition()
    , m_event_thread()
    , m_callback_list()
    , m_callback_mutex()
    , m_callback_condition()
    , m_callback_thread()
{
    m_client.clear_access_channels(websocketpp::log::alevel::all);
    m_client.clear_error_channels(websocketpp::log::elevel::all);
    m_client.init_asio();
}

template <typename client_type>
WebsocketSession<client_type>::~WebsocketSession()
{
    exit();
}

template <typename client_type>
bool WebsocketSession<client_type>::init(WebsocketClientSink * sink, const std::string & url)
{
    exit();

    m_sink = sink;
    m_url = url;

    m_client.set_close_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
        m_working = false;
        on_close();
    });

    m_client.set_fail_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
        m_working = false;
        on_close();
    });

    m_client.set_http_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
    });

    m_client.set_interrupt_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
        m_working = false;
        on_close();
    });

    m_client.set_message_handler([this](websocketpp::connection_hdl handle, websocketpp::config::asio_client::message_type::ptr message){
        set_handle(handle);
        if (nullptr != message)
        {
            const std::string & data = message->get_payload();
            bool binary = websocketpp::frame::opcode::BINARY == message->get_opcode();
            if (nullptr != m_sink && !data.empty())
            {
                m_sink->on_websocket_recv(data.data(), static_cast<uint32_t>(data.size()), binary);
            }
        }
    });

    m_client.set_open_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
        m_working = true;
        on_connect();
    });

    m_client.set_pong_handler([this](websocketpp::connection_hdl handle, std::string message){
        set_handle(handle);
    });

    m_client.set_tcp_post_init_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
    });

    m_client.set_tcp_pre_init_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
    });

    m_client.set_validate_handler([this](websocketpp::connection_hdl handle){
        set_handle(handle);
        return true;
    });

    set_specific_handler();

    m_running = true;

    m_event_thread = std::thread([this]{
        while (m_running)
        {
            std::list<bool> event_list;

            {
                std::unique_lock<std::mutex> locker(m_event_mutex);
                while (m_running && m_event_list.empty())
                {
                    m_event_condition.wait(locker);
                }
                event_list.swap(m_event_list);
            }

            for (std::list<bool>::const_iterator iter = event_list.begin(); event_list.end() != iter && m_running; ++iter)
            {
                if (*iter)
                {
                    do_connect();
                }
                else
                {
                    do_close();
                }
            }
        }
    });

    m_callback_thread = std::thread([this]{
        while (m_running)
        {
            std::list<bool> callback_list;

            {
                std::unique_lock<std::mutex> locker(m_callback_mutex);
                while (m_running && m_callback_list.empty())
                {
                    m_callback_condition.wait(locker);
                }
                callback_list.swap(m_callback_list);
            }

            for (std::list<bool>::const_iterator iter = callback_list.begin(); callback_list.end() != iter && m_running; ++iter)
            {
                if (nullptr == m_sink)
                {
                    continue;
                }
                if (*iter)
                {
                    m_sink->on_websocket_connect();
                }
                else
                {
                    m_sink->on_websocket_close();
                }
            }
        }
    });

    do
    {
        if (!m_event_thread.joinable())
        {
            RUN_LOG_ERR("websocket client init failure while create event thread failed");
            break;
        }

        if (!m_callback_thread.joinable())
        {
            RUN_LOG_ERR("websocket client init failure while create callback thread failed");
            break;
        }

        return true;
    } while (false);

    exit();

    return false;
}

template <typename client_type>
void WebsocketSession<client_type>::exit()
{
    if (m_running)
    {
        m_running = false;

        do_close();

        if (m_event_thread.joinable())
        {
            m_event_condition.notify_one();
            m_event_thread.join();
        }

        if (m_callback_thread.joinable())
        {
            m_callback_condition.notify_one();
            m_callback_thread.join();
        }

        if (m_work_thread.joinable())
        {
            m_work_thread.join();
        }

        m_event_list.clear();
        m_callback_list.clear();

        m_working = false;
    }
}

template <typename client_type>
void WebsocketSession<client_type>::connect()
{
    {
        std::lock_guard<std::mutex> locker(m_event_mutex);
        m_event_list.push_back(true);
    }
    m_event_condition.notify_one();
}

template <typename client_type>
void WebsocketSession<client_type>::close()
{
    {
        std::lock_guard<std::mutex> locker(m_event_mutex);
        m_event_list.push_back(false);
    }
    m_event_condition.notify_one();
}

template <typename client_type>
void WebsocketSession<client_type>::on_connect()
{
    {
        std::lock_guard<std::mutex> locker(m_callback_mutex);
        m_callback_list.push_back(true);
    }
    m_callback_condition.notify_one();
}

template <typename client_type>
void WebsocketSession<client_type>::on_close()
{
    {
        std::lock_guard<std::mutex> locker(m_callback_mutex);
        m_callback_list.push_back(false);
    }
    m_callback_condition.notify_one();
}

template <typename client_type>
void WebsocketSession<client_type>::on_error(const char * action, const char * message)
{
    if (nullptr != m_sink)
    {
        m_sink->on_websocket_error(action, message);
    }
}

template <typename client_type>
void WebsocketSession<client_type>::do_connect()
{
    if (m_running)
    {
        if (is_connected())
        {
            real_close();

            if (m_work_thread.joinable())
            {
                m_work_thread.join();
            }

            on_close();
        }
        else
        {
            if (m_work_thread.joinable())
            {
                m_work_thread.join();
            }
        }

        m_work_thread = std::thread([this]{
            real_connect();
            m_client.run();
        });
    }
    else
    {
        on_close();
    }
}

template <typename client_type>
void WebsocketSession<client_type>::do_close()
{
    if (is_connected())
    {
        real_close();

        if (m_work_thread.joinable())
        {
            m_work_thread.join();
        }

        on_close();
    }

    m_working = false;
}

template <typename client_type>
bool WebsocketSession<client_type>::send_message(const void * data, uint32_t size, bool binary)
{
    if (!m_running || !m_working || nullptr == data || 0 == size)
    {
        return false;
    }

    websocketpp::lib::error_code err;
    m_client.send(m_handle, data, size, binary ? websocketpp::frame::opcode::BINARY : websocketpp::frame::opcode::TEXT, err);
    if (err)
    {
        on_error("send", err.message().c_str());
        return false;
    }

    return true;
}

template <typename client_type>
bool WebsocketSession<client_type>::is_connected() const
{
    return m_working;
}

template <typename client_type>
websocketpp::client<client_type> & WebsocketSession<client_type>::get_client()
{
    return m_client;
}

template <typename client_type>
void WebsocketSession<client_type>::set_handle(websocketpp::connection_hdl handle)
{
    m_handle = handle;
}

template <typename client_type>
void WebsocketSession<client_type>::real_connect()
{
    try
    {
        if (m_client.stopped())
        {
            m_client.reset();
        }
        websocketpp::lib::error_code err;
        typename websocketpp::client<client_type>::connection_ptr conn = m_client.get_connection(m_url, err);
        if (err)
        {
            on_error("connect", err.message().c_str());
        }
        else
        {
            if (conn)
            {
                m_client.connect(conn);
            }
            else
            {
                on_error("connect", "unable to resolve address");
            }
        }
    }
    catch (websocketpp::lib::error_code err)
    {
        on_error("connect", err.message().c_str());
    }
    catch (const std::exception & e)
    {
        on_error("connect", e.what());
    }
    catch (...)
    {
        on_error("connect", "an unknown exception occurred");
    }
}

template <typename client_type>
void WebsocketSession<client_type>::real_close()
{
    try
    {
        websocketpp::lib::error_code err;
        m_client.close(m_handle, websocketpp::close::status::going_away, "", err);
        m_client.stop();
    }
    catch (websocketpp::lib::error_code err)
    {
        on_error("close", err.message().c_str());
    }
    catch (const std::exception & e)
    {
        on_error("close", e.what());
    }
    catch (...)
    {
        on_error("close", "an unknown exception occurred");
    }
}

class WebsocketSessionPlain : public WebsocketSession<websocketpp::config::asio_client>
{
private:
    virtual void set_specific_handler() override;
};

class WebsocketSessionSecure : public WebsocketSession<websocketpp::config::asio_tls_client>
{
private:
    virtual void set_specific_handler() override;
};


#endif // WEBSOCKET_CLIENT_IMPL_H
