/*
 * monitorroom1.h
 *
 *  Created on: Aug 27, 2015
 *      Author: root
 */

#ifndef MONITORROOM1_H_
#define MONITORROOM1_H_

#include "monitorparticipant.h"

class monitor_room
{
public:
  void join(monitor_participant_ptr participant)
  {
    participants_.insert(participant);
    for (auto msg: recent_msgs_)
      participant->deliver(msg);
  }

  void leave(monitor_participant_ptr participant)
  {
    participants_.erase(participant);
  }

  void deliver(message_t& msg)
  {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs){
      recent_msgs_.pop_front();
    }

    for (auto participant: participants_)
      participant->deliver(msg);
  }

private:
  std::set<monitor_participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  message_queue_t recent_msgs_;
};

#endif /* MONITORROOM1_H_ */
