/*
 * monitorparticipant1.h
 *
 *  Created on: Aug 27, 2015
 *      Author: root
 */

#ifndef MONITORPARTICIPANT1_H_
#define MONITORPARTICIPANT1_H_

class monitor_participant
{
public:
  virtual ~monitor_participant() {}
  virtual void deliver(message_t& msg) = 0;
};

typedef std::shared_ptr<monitor_participant> monitor_participant_ptr;

#endif /* MONITORPARTICIPANT1_H_ */
