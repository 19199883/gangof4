#include "quote_shfe_my.h"
#include "quote_cmn_utility.h"

#include "qtm.h"

#include "zmq.h"

using namespace std;
using namespace my_cmn;

#define ST_NORMAL 0
#define ST_NET_DISCONNECTED -1
#define ST_LOGIN_FAILED -2
#define ST_LOGOUT -3

// REP socket monitor thread
void *QuoteInterface_MY_SHFE_MD::rep_socket_monitor(void *vp, const std::string &pair_addr, int *cnn_counter)
{
    char qtm_name[32];
    sprintf(qtm_name, "shfe_my_%u", getpid());

    QuoteInterface_MY_SHFE_MD *p = (QuoteInterface_MY_SHFE_MD *) vp;
    void *ctx = p->zmq_context_;

    zmq_event_t event;
    int rc;

    void *s = zmq_socket(ctx, ZMQ_PAIR);
    if (!s)
    {
        MY_LOG_ERROR("create zmq socket of ZMQ_PAIR failed.");
        return NULL;
    }

    rc = zmq_connect(s, pair_addr.c_str());
    if (rc != 0)
    {
        QuoteUpdateState(qtm_name, QtmState::CONNECT_FAIL);
        MY_LOG_ERROR("connect ZMQ_PAIR failed.");
        return NULL;
    }
    while (true)
    {
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        rc = zmq_recvmsg(s, &msg, 0);
        if (rc == -1)
        {
            int e = zmq_errno();
            MY_LOG_ERROR("receive message failed, errno: %d", e);
            if (e == ETERM)
            {
                break;
            }
            continue;
        }
        memcpy(&event, zmq_msg_data(&msg), sizeof(event));
        switch (event.event)
        {
//            case ZMQ_EVENT_LISTENING:
//                MY_LOG_INFO("listening socket descriptor %d", event.data.listening.fd);
//                MY_LOG_INFO("listening socket address %s", event.data.listening.addr);
//                break;
//            case ZMQ_EVENT_ACCEPTED:
//                MY_LOG_INFO("accepted socket descriptor %d", event.data.accepted.fd);
//                MY_LOG_INFO("accepted socket address %s", event.data.accepted.addr);
//                break;
//            case ZMQ_EVENT_CLOSE_FAILED:
//                MY_LOG_INFO("socket close failure error code %d", event.data.close_failed.err);
//                MY_LOG_INFO("socket address %s", event.data.close_failed.addr);
//                break;
//            case ZMQ_EVENT_CLOSED:
//                MY_LOG_INFO("closed socket descriptor %d", event.data.closed.fd);
//                MY_LOG_INFO("closed socket address %s", event.data.closed.addr);
//                break;
            case ZMQ_EVENT_DISCONNECTED:
            {
//                MY_LOG_INFO("disconnected socket descriptor %d", event.data.disconnected.fd);
//                MY_LOG_INFO("disconnected socket address %s", event.data.disconnected.addr);

                boost::mutex::scoped_lock lock(p->status_sync_);
                *cnn_counter -= 1;
                MY_LOG_INFO("normal count: %d, %d, %d", p->provider_normal_set_.size(), p->mbl_connect_cnt_, p->md_connect_cnt_);
                QuoteUpdateState(qtm_name, QtmState::DISCONNECT);
            }
                break;
            case ZMQ_EVENT_CONNECTED:
            {
//                MY_LOG_INFO("connected socket descriptor %d", event.data.disconnected.fd);
//                MY_LOG_INFO("connected socket address %s", event.data.disconnected.addr);
                boost::mutex::scoped_lock lock(p->status_sync_);
                *cnn_counter += 1;
                MY_LOG_INFO("normal count: %d, %d, %d", p->provider_normal_set_.size(), p->mbl_connect_cnt_, p->md_connect_cnt_);
                if (p->provider_normal_set_.size() == (std::size_t)p->provider_cnt_cfg_
                    && p->mbl_connect_cnt_ == p->mbl_connect_cnt_cfg_
                    && p->md_connect_cnt_ == p->md_connect_cnt_cfg_)
                {
                    QuoteUpdateState(qtm_name, QtmState::API_READY);
                }
            }
                break;
        }
        zmq_msg_close(&msg);
    }
    zmq_close(s);
    return NULL;
}

QuoteInterface_MY_SHFE_MD::QuoteInterface_MY_SHFE_MD(const SubscribeContracts *subscribe_contracts, const ConfigData &cfg)
    : cfg_(cfg), p_shfe_deep_save_(NULL), p_shfe_ex_save_(NULL), p_my_shfe_md_save_(NULL),
        p_mbl_handler_(NULL), p_shfe_depth_md_handler_(NULL), p_shfe_status_handler_(NULL), my_shfe_md_inf_(cfg),
        t_mbl_socket_monitor_(NULL), t_md_socket_monitor_(NULL)
{
    if (subscribe_contracts)
    {
        subscribe_contracts_ = *subscribe_contracts;
    }

    if (subscribe_contracts_.empty())
    {
        MY_LOG_INFO("MY_SHFE_MD - subscribe all contract");
    }
    else
    {
        for (const std::string &v : subscribe_contracts_)
        {
            MY_LOG_INFO("MY_SHFE_MD - subscribe: %s", v.c_str());
        }
    }

    running_flag_ = true;

    sprintf(qtm_name_, "shfe_my_%u", getpid());
    QuoteUpdateState(qtm_name_, QtmState::INIT);

    p_shfe_deep_save_ = new QuoteDataSave<SHFEQuote>(cfg_, qtm_name_, "shfe_deep", SHFE_DEEP_QUOTE_TYPE, false);
    p_shfe_ex_save_ = new QuoteDataSave<CDepthMarketDataField>(cfg_, qtm_name_, "quote_level1", SHFE_EX_QUOTE_TYPE, false);
    p_my_shfe_md_save_ = new QuoteDataSave<MYShfeMarketData>(cfg_, qtm_name_, "my_shfe_md", MY_SHFE_MD_QUOTE_TYPE);

    mbl_connect_cnt_cfg_ = cfg.Logon_config().mbl_data_addrs.size();
    md_connect_cnt_cfg_ = cfg.Logon_config().depth_market_data_addrs.size();
    provider_cnt_cfg_ = cfg.Logon_config().status_data_addrs.size();
    mbl_connect_cnt_ = 0;
    md_connect_cnt_ = 0;

    // zmq context, one thread for one kind of quote data
    zmq_context_ = zmq_ctx_new();
    zmq_ctx_set(zmq_context_, ZMQ_IO_THREADS, 3);

    my_shfe_md_inf_.SetDataHandler(this);
    // start sub threads
    p_mbl_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::ShfeMBLHandler, this));
    p_shfe_depth_md_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::ShfeDepthMarketDataHandler, this));
    p_shfe_status_handler_ = new boost::thread(boost::bind(&QuoteInterface_MY_SHFE_MD::ProviderStatusHandler, this));
}

QuoteInterface_MY_SHFE_MD::~QuoteInterface_MY_SHFE_MD()
{
    // terminate all threads
    running_flag_ = false;
    if (p_mbl_handler_)
    {
        p_mbl_handler_->interrupt();
    }
    if (p_shfe_depth_md_handler_)
    {
        p_shfe_depth_md_handler_->interrupt();
    }
    if (p_shfe_status_handler_)
    {
        p_shfe_status_handler_->interrupt();
    }

    // destroy all save object
    if (p_shfe_deep_save_)
        delete p_shfe_deep_save_;

    if (p_shfe_ex_save_)
        delete p_shfe_ex_save_;

    if (p_my_shfe_md_save_)
        delete p_my_shfe_md_save_;
}

void QuoteInterface_MY_SHFE_MD::ShfeMBLHandler()
{
    void *zmq_sub = zmq_socket(zmq_context_, ZMQ_SUB);
    int snd_hwm = 300000;
    zmq_setsockopt(zmq_sub, ZMQ_RCVHWM, &snd_hwm, sizeof(snd_hwm));
    zmq_setsockopt(zmq_sub, ZMQ_SUBSCRIBE, NULL, 0);

    // start socket event monitor
    int rc = zmq_socket_monitor(zmq_sub, "inproc://mbl_monitor.rep", ZMQ_EVENT_ALL);
    MY_LOG_INFO("zmq_socket_monitor of mbl data, return: %d", rc);
    t_mbl_socket_monitor_ = new boost::thread(&QuoteInterface_MY_SHFE_MD::rep_socket_monitor, this, "inproc://mbl_monitor.rep", &mbl_connect_cnt_);

    for (const std::string &addr : cfg_.Logon_config().mbl_data_addrs)
    {
        zmq_connect(zmq_sub, addr.c_str());
    }

    SHFEQuote data;
    try
    {
        while (true)
        {
            int nbytes = zmq_recv(zmq_sub, &data, sizeof(SHFEQuote), 0);
            if (nbytes == sizeof(SHFEQuote))
            {
                timeval t;
                gettimeofday(&t, NULL);
                if (shfe_deep_data_handler_
                    && (subscribe_contracts_.empty()
                        || subscribe_contracts_.find(data.field.InstrumentID) != subscribe_contracts_.end()))
                {
                    shfe_deep_data_handler_(&data);
                }
                my_shfe_md_inf_.OnMBLData(&data.field, data.isLast);
                p_shfe_deep_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
            }
            else
            {
                MY_LOG_ERROR("MYShfeMBLHandler - receive data in wrong length: %d.", nbytes);
            }
        }
    }
    catch (...)
    {
        MY_LOG_FATAL("Unknown exception in MYShfeMBLHandler.");
    }
}

void QuoteInterface_MY_SHFE_MD::ShfeDepthMarketDataHandler()
{
    void *zmq_sub = zmq_socket(zmq_context_, ZMQ_SUB);
    int snd_hwm = 300000;
    zmq_setsockopt(zmq_sub, ZMQ_RCVHWM, &snd_hwm, sizeof(snd_hwm));

    // start socket event monitor
    int rc = zmq_socket_monitor(zmq_sub, "inproc://md_monitor.rep", ZMQ_EVENT_ALL);
    MY_LOG_INFO("zmq_socket_monitor of depth market data, return: %d", rc);
    t_md_socket_monitor_ = new boost::thread(&QuoteInterface_MY_SHFE_MD::rep_socket_monitor, this, "inproc://md_monitor.rep", &md_connect_cnt_);

    for (const std::string &addr : cfg_.Logon_config().depth_market_data_addrs)
    {
        zmq_connect(zmq_sub, addr.c_str());
    }
    zmq_setsockopt(zmq_sub, ZMQ_SUBSCRIBE, NULL, 0);

    CDepthMarketDataField data;
    try
    {
        while (true)
        {
            int nbytes = zmq_recv(zmq_sub, &data, sizeof(CDepthMarketDataField), 0);
            if (nbytes == sizeof(CDepthMarketDataField))
            {
                // filter duplicate datas
                MY_LOG_DEBUG("filter duplicate datas begin");
                DepthMarketDataID id(data);
                if (md_ids_.find(id) == md_ids_.end())
                {
                    md_ids_.insert(id);
                    MY_LOG_DEBUG("filter duplicate datas end");
                    timeval t;
                    gettimeofday(&t, NULL);
                    if (shfe_ex_handler_
                        && (subscribe_contracts_.empty()
                            || subscribe_contracts_.find(data.InstrumentID) != subscribe_contracts_.end()))
                    {
                        shfe_ex_handler_(&data);
                    }
                    my_shfe_md_inf_.OnDepthMarketData(&data);
                    p_shfe_ex_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, &data);
                }
            }
            else
            {
                MY_LOG_ERROR("MYShfeDepthMarketDataHandler - receive data in wrong length: %d.", nbytes);
            }
        }
    }
    catch (...)
    {
        MY_LOG_FATAL("Unknown exception in MYShfeDepthMarketDataHandler.");
    }
}
void QuoteInterface_MY_SHFE_MD::OnMYShfeMDData(MYShfeMarketData *data)
{
    timeval t;
    gettimeofday(&t, NULL);
    if (my_shfe_md_handler_
        && (subscribe_contracts_.empty()
            || subscribe_contracts_.find(data->InstrumentID) != subscribe_contracts_.end()))
    {
        my_shfe_md_handler_(data);
    }
    p_my_shfe_md_save_->OnQuoteData(t.tv_sec * 1000000 + t.tv_usec, data);
}

void QuoteInterface_MY_SHFE_MD::ProviderStatusHandler()
{
    void *zmq_sub = zmq_socket(zmq_context_, ZMQ_SUB);
    int snd_hwm = 1000;
    zmq_setsockopt(zmq_sub, ZMQ_RCVHWM, &snd_hwm, sizeof(snd_hwm));
    for (const std::string &addr : cfg_.Logon_config().status_data_addrs)
    {
        zmq_connect(zmq_sub, addr.c_str());
    }
    zmq_setsockopt(zmq_sub, ZMQ_SUBSCRIBE, NULL, 0);

    ShfeProviderStatus data;
    try
    {
        while (true)
        {
            int nbytes = zmq_recv(zmq_sub, &data, sizeof(ShfeProviderStatus), 0);
            if (nbytes == sizeof(ShfeProviderStatus))
            {
                boost::mutex::scoped_lock lock(status_sync_);
                MY_LOG_INFO("recv provider report, id=%d, state=%d, desc=%s", data.id, data.status, data.desc);

                if (data.status == ST_NORMAL)
                {
                    provider_normal_set_.insert(data.id);
                    MY_LOG_INFO("normal count: %d, %d, %d", provider_normal_set_.size(), mbl_connect_cnt_, md_connect_cnt_);
                    if (provider_normal_set_.size() == (std::size_t)provider_cnt_cfg_
                        && mbl_connect_cnt_ == mbl_connect_cnt_cfg_
                        && md_connect_cnt_ == md_connect_cnt_cfg_)
                    {
                        MY_LOG_INFO("update_state, state=%d, desc=%s", ST_NORMAL, "connect to provider and provider normal.");
                        QuoteUpdateState(qtm_name_, QtmState::API_READY);
                    }
                }
                else
                {
                    provider_normal_set_.erase(data.id);
                    MY_LOG_INFO("update_state, state=%d, desc=%s", data.status, data.desc);
                    char err_msg[64];
                    sprintf(err_msg, "data received with abnormal state: %d", data.status);
                    update_state(qtm_name_, TYPE_QUOTE, QtmState::UNDEFINED_API_ERROR, err_msg);
                    MY_LOG_INFO("update_state: name: %s, State: %d, Description: %s.", qtm_name_, QtmState::UNDEFINED_API_ERROR, err_msg);
                }
            }
            else
            {
                MY_LOG_ERROR("ProviderStatusHandler - receive data in wrong length: %d.", nbytes);
            }
        }
    }
    catch (...)
    {
        MY_LOG_FATAL("Unknown exception in ProviderStatusHandler.");
    }
}
