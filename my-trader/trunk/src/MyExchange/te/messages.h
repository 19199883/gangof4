//
// messages.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "option_interface.h"

enum main_cmd_enum_t{
	get = 'G',
	update = 'U',
};

enum sub_cmd_enum_t{
	update_init = '1',
	updae_update = '2',
	get_monitor=0xf0,
};

enum frame_component_t{
	tail = 0xff,
};

enum ack_enum_t{
	monitor = 'M',
};

#pragma pack(push)
#pragma pack(1)

typedef struct _cmd_init {
	char strategy_name[16];
	char parameters[1024];
	startup_init_t content;
} cmd_init_t;

typedef struct _cmd_update {
	char strategy_name[16];
	char parameters[1024];
	ctrl_t content;
} cmd_update_t;

typedef struct _cmd_get {
	char strategy_name[16];
	char parameters[1024];
} cmd_get_t;


typedef struct _ack_fail {
	char parameters[1024];
} ack_fail_t;

typedef struct _ack_monitor {
	monitor_t data;
} ack_monitor_t;

#pragma pack(pop)


class message_t
{
public:
  enum { header_length = 7 };
  enum { max_body_length = 80 * 1024 };

  message_t()
    : body_length_(0)
  {
  }

  const char* data() const
  {
    return data_;
  }

  char* data()
  {
    return data_;
  }

  std::size_t length() const
  {
    return header_length + body_length_;
  }

  const char* body() const
  {
    return data_ + header_length;
  }

  char* body()
  {
    return data_ + header_length;
  }

  std::size_t body_length() const
  {
    return body_length_;
  }

  void body_length(std::size_t new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length){
      body_length_ = max_body_length;
    }
  }

  bool decode_header()
  {
	size_t id_size = sizeof(id);
	std::memcpy(&id, data_, id_size);

	size_t cmd_size = sizeof(cmd);
	std::memcpy(&cmd, data_+id_size, cmd_size);

	size_t len_size = sizeof(body_length_);
	std::memcpy(&body_length_, data_+id_size+cmd_size, len_size);

	if (body_length_ > max_body_length)
	{
		body_length_ = 0;
		return false;
	}
	return true;
  }

  void encode_header()
  {
	  size_t id_size = sizeof(id);
	  std::memcpy(data_, &id, id_size);

	  size_t cmd_size = sizeof(cmd);
	  std::memcpy(data_+id_size, &cmd, cmd_size);

	  size_t len_size = sizeof(body_length_);
	  std::memcpy(data_+id_size+cmd_size, &body_length_, len_size);
  }
public:
  uint16_t id;
  uint8_t cmd;

private:
  char data_[header_length + max_body_length];
  uint32_t body_length_;
};

typedef std::deque<message_t> message_queue_t;


#endif // MESSAGES_HPP
