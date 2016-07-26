/*
 * monitorsession1.h
 *
 *  Created on: Aug 27, 2015
 *      Author: root
 */

#ifndef MONITORSESSION1_H_
#define MONITORSESSION1_H_

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <stdio.h>
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
#include <mutex>
#include "monitorparticipant.h"
#include "strategy_unit.h"
#include <memory>
#include "quote_entity.h"
#include "option_interface.h"
#include "rssmonitorbase.h"
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/logger.h>

using boost::asio::ip::tcp;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

class monitor_session
  : public monitor_participant,
    public std::enable_shared_from_this<monitor_session>
{
public:
  monitor_session(tcp::socket socket, monitor_room& room,std::shared_ptr<StrategyT> strategy)
    : socket_(std::move(socket)),
      room_(room),
      strategy_(strategy)
  {
  }

  void start()
  {
    room_.join(shared_from_this());
    do_read_header();
  }

  static void create(uint16_t req_id,uint8_t err,const ack_fail_t& ack,message_t &new_msg)
  {
	  new_msg.id = req_id;
	  new_msg.cmd = err;
	  new_msg.body_length(sizeof(ack));
	  std::memcpy(new_msg.body(), &ack, new_msg.body_length());
  }

  static void create(uint16_t req_id,const ack_monitor_t& ack,message_t &new_msg)
  {
	  new_msg.id = req_id;
	  new_msg.cmd = ack_enum_t::monitor;
	  new_msg.body_length(sizeof(ack));
	  std::memcpy(new_msg.body(), &ack, new_msg.body_length());
  }

  static void create_ack_ok(uint16_t req_id,message_t &new_msg)
  {
	  new_msg.id = req_id;
	  new_msg.cmd = 0;
	  new_msg.body_length(0);
  }


  void deliver(message_t& msg)
  {
	  LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
			  "[OP monitor]:enter deliver");

	  // invoke model's methods
	  message_t ack_msg;
	  memset (&ack_msg, 0, sizeof(ack_msg));

	  ack_fail_t ack_fail;
	  memset (&ack_fail, 0, sizeof(ack_fail));

	  ack_monitor_t monitor_info;
	  memset (&monitor_info, 0, sizeof(monitor_info));

	  int err = 0;
	  switch (msg.cmd){
	  	  case (uint8_t)main_cmd_enum_t::update:
	  		  {
				LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							  "[OP monitor]:deliver,update");

				cmd_init_t *init_cmd = (cmd_init_t *)(msg.body());
				if(strcmp(init_cmd->parameters,"type=1")==0){// initialize strategy
					LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							"[OP monitor]:deliver,update,init");
					init_strategy(0,&(init_cmd->content),&err);
					if(0 == err){// success
						monitor_session::create_ack_ok(msg.id,ack_msg);
						LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							"[OP monitor]:deliver,update,init,ok");
					}else{
						monitor_session::create(msg.id,err,ack_fail,ack_msg);
						LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							"[OP monitor]:deliver,update,init,failure");
					}
				}else if(0==strcmp(init_cmd->parameters,"type=2")){// update parameters if strategy periodically
					LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							"[OP monitor]:deliver,update,update");
					cmd_update_t *update_cmd = (cmd_update_t *)(msg.body());
					update_strategy(0,&(update_cmd->content),&err);
					if(0 == err){// success
						monitor_session::create_ack_ok(msg.id,ack_msg);
						LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
								"[OP monitor]:deliver,update,update,ok");
					}else{
						monitor_session::create(msg.id,err,ack_fail,ack_msg);
						LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
								"[OP monitor]:deliver,update,update,failure");
					}
				}
	  		  }
	  	  break;
	  	case (uint8_t)main_cmd_enum_t::get:
	  		{
	  			LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
	  				"[OP monitor]:deliver,update,get");
	  			cmd_get_t *get_cmd = (cmd_get_t *)(msg.body());
				if(0 == strcmp(get_cmd->parameters,"type=0xf0")){// get monitoring information
					LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
						  	"[OP monitor]:deliver,update,get");
					int err = get_monitor_info(0,&(monitor_info.data),&err);
					if(0 == err){// success
						monitor_session::create(msg.id,monitor_info,ack_msg);
						LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							"[OP monitor]:deliver,update,get,ok");
					}else{
						monitor_session::create(msg.id,err,ack_fail,ack_msg);
						LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
							"[OP monitor]:deliver,update,get,failure");
					}
				}
	  		}
	  		break;
	  	default:
	  		return;
	  }

	  ack_msg.encode_header();

	  rw_lock_.lock();
	  write_msgs_.push_back(ack_msg);
	  rw_lock_.unlock();

	  do_write();
  }

private:

  int init_strategy(uint16_t strategy_id, startup_init_t *init_content, int *err)
  {
	  return strategy_->InitOptionStrategy(init_content,err);
  }

  int update_strategy(uint16_t strategy_id, ctrl_t *update_content, int *err)
  {
	  return strategy_->UpdateOptionStrategy(update_content,err);
  }

  int get_monitor_info(uint16_t strategy_id, monitor_t *monitor_content, int *err)
   {
	  return strategy_->GetMonitorInfo(monitor_content,err);
   }

  void do_read_header()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.data(), message_t::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header()){
            do_read_body();
          }else{
            room_.leave(shared_from_this());
          }
          LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
        		  "[OP monitor]:session,do_read_header," << ec);
        });
  }

  void do_read_body()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()+1),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec){
            room_.deliver(read_msg_);
            do_read_header();
          }else{
            room_.leave(shared_from_this());
          }
          LOG4CXX_TRACE(log4cxx::Logger::getRootLogger(),
         		"[OP monitor]:session,do_read_header," << ec);
        });
  }

  void do_write()
  {
	  rw_lock_.lock();
	  message_t msg_to_wr = write_msgs_.front();
	  write_msgs_.pop_front();
	  rw_lock_.unlock();

	  boost::asio::async_write(socket_,
        boost::asio::buffer(msg_to_wr.data(),
        		msg_to_wr.length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec){
        	  // write end of frame(eof)
        	  char tail = frame_component_t::tail;
        	  boost::asio::write(socket_,boost::asio::buffer(&tail,1));

        	  rw_lock_.lock();
        	  bool empty = write_msgs_.empty();
        	  rw_lock_.unlock();

				if (!empty){
				  do_write();
				}
			  }else{
				room_.leave(shared_from_this());
			  }
        });
  }

  tcp::socket socket_;
  monitor_room& room_;
  message_t read_msg_;
  message_queue_t write_msgs_;
  std::mutex rw_lock_;
  std::shared_ptr<StrategyT> strategy_;
};


#endif /* MONITORSESSION1_H_ */
