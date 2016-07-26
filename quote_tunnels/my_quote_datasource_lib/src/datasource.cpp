#include "datasource.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace my_cmn;

static bool ParseConfig(const ConfigData &cfg, std::string &addr_ip_, std::string &port_)
{
    // 行情服务地址配置
    if (cfg.Logon_config().quote_provider_addrs.empty())
    {
        MY_LOG_ERROR("MYStock - havnt configure quote provider's address.");
        return false;
    }
    // 主服务地址
    {
        IPAndPortStr main_cfg = ParseIPAndPortStr(cfg.Logon_config().quote_provider_addrs.front());
        if (main_cfg.first.empty() || main_cfg.second.empty())
        {
            return false;
        }
        addr_ip_ = main_cfg.first;
        port_ = main_cfg.second;
    }

    return true;
}

QuoteInterface_MY_STOCK::QuoteInterface_MY_STOCK(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : reconnect_timer_(io_service_),
        socket_(io_service_),
        strand_(io_service_),
        cfg_(cfg),
        p_save_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    sprintf(qtm_name_, "my_datasource_%u", getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    io_serv_t = NULL;
    interupt_seconds_ = 0;
    // 连接服务
    if (!ParseConfig(cfg_, server_ip_, server_port_))
    {
        return;
    }

    header_ = new my_datasource_header();

    SetInitConnectStatus();

    // 成员初始化，按协议约定，不做配置项
    reconnect_miliseconds_ = 3000;

    p_save_ = new QuoteDataSave<my_datasource_data>(cfg_, qtm_name_, "my_datasource", MY_STOCK_QUOTE_TYPE, false);

    // 接入点解析
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(server_ip_, server_port_);
    iter_endpoint_ = resolver.resolve(query);

    // 启动连接
    boost::asio::async_connect(socket_, iter_endpoint_,
        boost::bind(&QuoteInterface_MY_STOCK::HandleConnect, this,
            boost::asio::placeholders::error));

    io_serv_t = new boost::thread(
        boost::bind((std::size_t (boost::asio::io_service::*)())&boost::asio::io_service::run,
            boost::ref(io_service_)));

    // TODO server should support heartbeat
    heatbeat_check_t_ = NULL;
    //heatbeat_check_t_ = new boost::thread(boost::bind(&QuoteInterface_MY_STOCK::HeartbeatCheck, this));
}

void QuoteInterface_MY_STOCK::StartConnect(const boost::system::error_code& error)
{
    if (!error)
    {
        boost::asio::async_connect(socket_, iter_endpoint_,
            boost::bind(&QuoteInterface_MY_STOCK::HandleConnect, this,
                boost::asio::placeholders::error));
    }
}

void QuoteInterface_MY_STOCK::HandleConnect(const boost::system::error_code& error)
{
    if (!error)
    {
        // 启动读处理
        boost::asio::async_read(socket_,
            boost::asio::buffer(&array_buffer_[0], sizeof(my_datasource_header)),
            strand_.wrap(boost::bind(&QuoteInterface_MY_STOCK::HeaderReadHandle, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)));

        SetConnectedStatus();
    }
    else
    {
        SetInitConnectStatus();

        // 服务器未启动时，将不断重连，导致日志信息太多
        RecordError(error);
        StartReconnectTimer();
    }
}

void QuoteInterface_MY_STOCK::HeaderReadHandle(const boost::system::error_code& ec,
    std::size_t bytes_transferred)
{
    if (!ec)
    {
        interupt_seconds_ = 0;
        my_datasource_header* header = (my_datasource_header*) &array_buffer_[0];
        memcpy(header_, header, sizeof(my_datasource_header));
        int bodylen = header_->nBodyLength;
        //MY_LOG_INFO("recv header, length:%d", bodylen);
        boost::asio::async_read(socket_,
            boost::asio::buffer(&array_buffer_[0], bodylen),
            strand_.wrap(boost::bind(&QuoteInterface_MY_STOCK::BodyReadHandle, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)));
    }
    else
    {
        SetInitConnectStatus();
        MY_LOG_ERROR("MYStock - quote provider disconnected when read head.");
        RecordError(ec);
        StartReconnectTimer();
    }
}

void QuoteInterface_MY_STOCK::BodyReadHandle(const boost::system::error_code& ec,
    std::size_t bytes_transferred)
{
    if (!ec)
    {
        // parse message
        if (bytes_transferred == sizeof(my_datasource_data))
        {
            my_datasource_data* p = (my_datasource_data*) &array_buffer_[0];
            timeval t;
            gettimeofday(&t, NULL);

            if (quote_data_handler_)
            {
                quote_data_handler_(p);
            }

            // 存起来
            p_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, p);
        }
        else
        {
            MY_LOG_WARN("MYStock - receive data in wrong length: %d", bytes_transferred);
        }

        boost::asio::async_read(socket_,
            boost::asio::buffer(&array_buffer_[0], sizeof(my_datasource_header)),
            strand_.wrap(boost::bind(&QuoteInterface_MY_STOCK::HeaderReadHandle, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)));
    }
    else
    {
        SetInitConnectStatus();
        MY_LOG_ERROR("MYStock - quote provider disconnected when read body.");
        RecordError(ec);
        StartReconnectTimer();
    }
}

void QuoteInterface_MY_STOCK::StartReconnectTimer()
{
    try
    {
        // 重连定时器开始
        reconnect_timer_.expires_from_now(boost::posix_time::millisec(reconnect_miliseconds_));

        // 绑定定时器执行函数
        reconnect_timer_.async_wait(
            boost::bind(&QuoteInterface_MY_STOCK::StartConnect, this,
                boost::asio::placeholders::error));
    }
    catch (...)
    {
        MY_LOG_ERROR("MYStock - error in StartReconnectTimer.");
    }
}

int QuoteInterface_MY_STOCK::Send(const std::string &message)
{
    if (IsConnected())
    {
        DoSend(message);
        return 0;
    }

    return -1;
}

void QuoteInterface_MY_STOCK::Close()
{
    if (socket_.is_open())
    {
        (void) socket_.close(unuse_ec_);
    }
}

void QuoteInterface_MY_STOCK::SendFinished(const boost::system::error_code& error)
{
    if (!error)
    {
        // 发送完成，无需额外处理
    }
    else
    {
        RecordError(error);
    }
}

void QuoteInterface_MY_STOCK::DoSend(const std::string &message)
{
    boost::mutex::scoped_lock lock(send_sync_);

    str_msg_ = message;

    // 异步发送
    boost::asio::async_write(socket_,
        boost::asio::buffer(str_msg_.data(), str_msg_.length()),
        strand_.wrap(
            boost::bind(&QuoteInterface_MY_STOCK::SendFinished, this,
                boost::asio::placeholders::error)));

    MY_LOG_DEBUG("MYStock - send to %s: %s", peer_info_.c_str(), str_msg_.c_str());
}

void QuoteInterface_MY_STOCK::RecordError(const boost::system::error_code& error)
{
    try
    {
        MY_LOG_ERROR("MYStock - error of %s: %s", peer_info_.c_str(), boost::system::system_error(error).what());
    }
    catch (...)
    {
        // 析构时，资源已经释放，线程运行时异常
    }
}

void QuoteInterface_MY_STOCK::SetInitConnectStatus()
{
    boost::mutex::scoped_lock lock(status_mutex_);
    connected_ = false;
    connect_info_ = "init status";

    QuoteUpdateState(qtm_name_, QtmState::INIT);
}

void QuoteInterface_MY_STOCK::SetConnectedStatus()
{
    boost::mutex::scoped_lock lock(status_mutex_);
    connected_ = true;

    peer_info_ = socket_.remote_endpoint().address().to_string();
    peer_info_.append(":");
    peer_info_.append(boost::lexical_cast<std::string>(socket_.remote_endpoint().port()));

    connect_info_ = "connect to ";
    connect_info_.append(peer_info_);
    MY_LOG_INFO("MYStock - quote provider connected: %s", connect_info_.c_str());

    boost::system::error_code error_code;
    boost::asio::ip::tcp::no_delay option(true);
    socket_.set_option(option, error_code);
    if (error_code)
    {
        RecordError(error_code);
    }

    QuoteUpdateState(qtm_name_, QtmState::API_READY);
}

QuoteInterface_MY_STOCK::~QuoteInterface_MY_STOCK()
{
    if (io_serv_t)
    {
        io_serv_t->yield();
        io_serv_t->interrupt();
    }
    io_service_.stop();

    Close();
    reconnect_timer_.cancel();

    if (p_save_)
        delete p_save_;
}

void QuoteInterface_MY_STOCK::HeartbeatCheck()
{
    while (true)
    {
        if (interupt_seconds_ > 30)
        {
            bool connect_tmp;
            {
                boost::mutex::scoped_lock lock(status_mutex_);
                connect_tmp = connected_;
            }
            if (connect_tmp)
            {
                MY_LOG_ERROR("MYStock - no heartbeat message for 30 seconds, try to reconnect.");
                SetInitConnectStatus();

                boost::asio::async_connect(socket_, iter_endpoint_,
                    boost::bind(&QuoteInterface_MY_STOCK::HandleConnect, this,
                        boost::asio::placeholders::error));
            }
            interupt_seconds_ = 0;
        }

        sleep(1);
        ++interupt_seconds_;
    }
}
