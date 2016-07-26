#if !defined(MULTI_PKG_STRUCT_H)
#define MULTI_PKG_STRUCT_H

#include <iostream>

using namespace std;

#pragma pack(push)
#pragma pack(8)

/* package head  extract*/
#define PKG_SIZE_OFFSET 4
#define PKG_SIZE_LEN 2 

#define MSG_CNT_OFFSET 6
#define MSG_CNT_LEN 1

#define PKG_SEQ_NUM_OFFSET 7
#define PKG_SEQ_NUM_LEN 8

#define CHANNEL_ID_OFFSET 15
#define CHANNEL_ID_LEN 1

#define CHANNEL_TYPE_OFFSET 16
#define CHANNEL_TYPE_LEN 1

#define PKG_HEAD_LEN 32 
/* package head  extract END*/

/* message head */
#define MSG_TYPE_OFFSET 0
#define MSG_TYPE_LEN 2

#define MSG_SIZE_OFFSET 2
#define MSG_SIZE_LEN 2

#define  MSG_HEAD_LEN 4
/* message head END*/

/* message quote snapshot  */
#define SNAP_SEQ_NUM_OFFSET 0
#define SNAP_SEQ_NUM_LEN 4

#define SNAP_CONTR_SET_OFFSET 4
#define SNAP_CONTR_SET_LEN 51 

#define SNAP_UPDATE_TIME_OFFSET 55
#define SNAP_UPDATE_TIME_LEN 8

#define SNAP_FIELD_CNT_OFFSET 63
#define SNAP_FIELD_CNT_LEN 1

#define SNAP_QUOTE_AR_OFFSET 64
#define SNAP_QUOTE_AR_EACH_LEN 9
#define SNAP_QUOTE_FID_MEAN 1 
/*message quote snapshot END */

#define RECV_BUF_LEN 1500

/*item like : SR 、ZC ...*/
#define MAX_ITEM_LEN 16
#define MAX_YEAR_LEN 4
#define MAX_MONTH_LEN 4
#define MAX_CONTR_LEN 32 
#define MAX_CONTR_TYPE_LEN 16

#define F_CONTR_SET_TYPE_IDX 1
#define F_CONTR_SET_ITEM_IDX 2 
#define F_CONTR_SET_DATE_IDX 3 
#define F_CONTR_SET_END_DATE_IDX 4
#define CONNECT_CAL_SPR_ABR_LEN 1

#define OUT_LEVER_NUM 5
#define FIELD_LEN 8
#define STR_CMP_NUM 1
#define MSG_RECOVERY_END_LEN 4

/* struct pkg_head total  32 byte*/
typedef struct pkg_head {
	char flags[3];
	unsigned char version;
	unsigned char msg_count;
	unsigned char channel_id;
	unsigned char channel_type;
	unsigned short int pkg_size; /*just include pkg_body size*/
	unsigned long long seq_num; /*pkg seq , start from realtime channel*/
	char resrved[15];
}pkg_head_t; 

typedef struct quote_field {
	double pre_close_price; 
	double pre_settle_price;
	unsigned long long pre_pos_qty;
	
	double open_price;
	double last_price;
	double high_price;
	double low_price;
	
	double his_high_price;
	double his_low_price;
	
	double lim_up_price;
	double lim_down_price;
	
	unsigned long long tot_match_qty;
	unsigned long long pos_qty;
	double aver_price;
	double close_price;
	double settle_price;
	
	unsigned long long last_qty;

	double bid_price[20];
	unsigned long long bid_qty[20];
	double ask_price[20];
	unsigned long long ask_qty[20];
	
	double implied_bid_price;
	unsigned long long implied_bid_qty;
	double implied_ask_price;
	unsigned long long implied_ask_qty;

	unsigned long long pre_delta;
	unsigned long long curr_delta;

	unsigned long long inside_qty;
	unsigned long long outside_qty;

	unsigned long long tot_bid_qty;
	unsigned long long tot_ask_qty;

	double tot_turn_over;
}quote_field_t;


typedef struct  quote_snap_shot {
	unsigned char field_cnt;
	char contr_type; /* 'F' -> FUTURES , 'O'->OPTIONS */
	unsigned int seq_num; /*snap shot seq , unique for different contract */
	char contr[MAX_CONTR_LEN];
 	unsigned long long update_time;
 	quote_field_t quote_field;
 	
	// HH:MM:SS.mmm
	std::string GetQuoteTime() const
	{
	   return std::string();
	}

}quote_snap_shot_t;

/*struct  msg_head total 4 byte */
typedef struct  msg_head {
	unsigned short int msg_type; /* 0x1001 QuoteSnapshot */
	unsigned short int msg_size; /* just include msg body len  */
}msg_head_t;

/* 
msg_body : 
		when msg_head msg_type=0x2001 equal 4 byte,
		when  msg_type = 0x1001 equal (64 + field_cnt*sizeof(quote_field_t)) 
*/
typedef struct msg_body {
	union msg_body_u{
		unsigned int contr_total;
		quote_snap_shot_t msg_body_snap_shot;
	};
}msg_body_t;

typedef struct msg{
	msg_head_t msg_hd;
	msg_body_t msg_db;
}msg_t;

typedef struct multi_pkg {
	pkg_head_t pkg_hd;
	msg_t msg[0];
}multi_pkg_t;


#pragma pack(pop)

#endif 
 /* MULTI_PKG_STRUCT_H */ 

