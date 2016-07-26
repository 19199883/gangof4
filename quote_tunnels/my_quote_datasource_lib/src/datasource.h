#pragma once

#include <string>
#include <unordered_map>

#include <boost/function.hpp>

#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/functional.hpp>

#include "quote_interface_datasource.h"
#include "my_cmn_util_funcs.h"
#include "quote_cmn_utility.h"
#include "quote_cmn_config.h"
#include "quote_cmn_save.h"

class QuoteInterface_MY_STOCK
: private boost::noncopyable,
    public boost::enable_shared_from_this<QuoteInterface_MY_STOCK>
{
public:
    // 构造函数
    QuoteInterface_MY_STOCK(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg);

    // 数据处理回调函数设置
    void SetQuoteDataHandler(boost::function<void(const my_datasource_data *)> quote_data_handler)
    {
        quote_data_handler_ = quote_data_handler;
    }

    // 连接状态
    virtual bool IsConnected() const
    {
        boost::mutex::scoped_lock lock(status_mutex_);
        return connected_;
    }

    // 连接的信息
    virtual std::string ConnectInfo() const
    {
        boost::mutex::scoped_lock lock(status_mutex_);
        return peer_info_;
    }

    // 消息发送接口, 返回错误码
    virtual int Send(const std::string &message);

    virtual ~QuoteInterface_MY_STOCK();

private:
    // 连接
    void StartConnect(const boost::system::error_code& error);

    // 连接事件处理
    void HandleConnect(const boost::system::error_code& error);

    // 连接状态信息设置
    void SetInitConnectStatus();
    void SetConnectedStatus();

    void StartReconnectTimer();

    void Close();

    // 错误记录
    void RecordError(const boost::system::error_code& error);

    void DoSend(const std::string &message);

    void SendFinished(const boost::system::error_code& error);

    void HeaderReadHandle(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void BodyReadHandle(const boost::system::error_code& ec, std::size_t bytes_transferred);

private:
    boost::asio::io_service io_service_;

    int reconnect_miliseconds_;
    boost::asio::deadline_timer reconnect_timer_;

    // 从缓存区取出数据
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver::iterator iter_endpoint_;

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::io_service::strand strand_;

    boost::asio::streambuf stream_buffer_;

    boost::array<char, 10 * sizeof(my_datasource_data)> array_buffer_;
    my_datasource_header *header_;

    // 异步发送，需要保证缓存区的生命周期，所以使用成员变量
    std::string str_msg_;
    boost::system::error_code unuse_ec_;

    // 连接状态
    std::string server_ip_, server_port_;
    volatile bool connected_;
    std::string connect_info_;
    std::string peer_info_;
    boost::thread *io_serv_t;

    // 同步对象
    boost::mutex send_sync_;
    mutable boost::mutex status_mutex_;

    // 数据处理函数对象
    boost::function<void(const my_datasource_data *)> quote_data_handler_;

    // 订阅合约集合
    SubscribeContracts subscribe_contracts_;

    // 配置数据对象
    ConfigData cfg_;

    // save assistant object
    QuoteDataSave<my_datasource_data> *p_save_;

    boost::atomic<int> interupt_seconds_;
    boost::thread *heatbeat_check_t_;
    void HeartbeatCheck();

    char qtm_name_[32];
};
