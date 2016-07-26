/*
 * proxy.h
 *
 *  Created on: Aug 21, 2015
 *      Author: root
 */

#ifndef PROXY_H_
#define PROXY_H_

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "messages.h"
#include <memory>
#include <vector>
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include "monitorparticipant.h"
#include "monitorroom.h"
#include "messages.h"
#include "monitorsession.h"
#include "strategy_unit.h"
#include <memory>
#include "quote_entity.h"
#include "rssmonitorbase.h"

using boost::asio::ip::tcp;
using namespace trading_engine;



class monitor_proxy : rss_monitor_base
{
public:
  monitor_proxy(boost::asio::io_service& io_service, const tcp::endpoint& endpoint)
    : acceptor_(io_service, endpoint),
      socket_(io_service)
  {

  }

  void start(shared_ptr<StrategyT> strategy)
   {
	  LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
	  		"[OP monitor]:proxy,start");
	  strategy_ = strategy;
	  do_accept();
   }

   void stop()
   {

   }

private:
  void do_accept()
  {
	  LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
	    "[OP monitor]:proxy,enter do_accept");

    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec)
        {
          if (!ec){
            std::make_shared<monitor_session>(std::move(socket_), room_,strategy_)->start();
          }
//          LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
//                "[OP monitor]:proxy,start," << ec);
          do_accept();
        });
  }

  shared_ptr<StrategyT> strategy_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  monitor_room room_;
};


#endif /* PROXY_H_ */
