#ifndef __TOE_APP_H_
#define __TOE_APP_H_

#include <pthread.h>

#define __SEMAPHORE__     1

#ifdef __SEMAPHORE__
#include <semaphore.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif	

#define DRV_MYFPGA        "/dev/myfpga"

#define IOCTL_INIT        0x8039
#define IOCTL_CONNECT     0x8040
#define IOCTL_LISTEN      0x8041
#define IOCTL_CLOSE       0x8042
#define IOCTL_RECV        0x8043
#define IOCTL_SEND        0x8044
#define IOCTL_CONFIG      0x8045
#define IOCTL_POLL        0x8046     /* 返回内存首地址，记录的原ip，目的ip，sid-fd-pid关联信息 */
#define IOCTL_LATENCY     0x8047     /* 用于测时延 */
#define IOCTL_HIJACK      0x8048
#define IOCTL_SET_IP      0x8049
#define IOCTL_SET_MAC     0x8050



/* Session connecting */
#define CONNECTING  1

/* Port in use */
#define EPORTINUSE  55

/* No free session */
#define ENOFREE     66

#define MAX_SESSION_NUM        16
#define MAX_TOE_NUM            2

#define ZONE_ENTRY_LEN         2048
#define ZONE_ENTRY_TX_NUM      64
#define ZONE_TOTAL_TX_LEN      (ZONE_ENTRY_TX_NUM*ZONE_ENTRY_LEN)
#define ZONE_ENTRY_RX_NUM      2048
#define ZONE_TOTAL_RX_LEN      (ZONE_ENTRY_RX_NUM*ZONE_ENTRY_LEN)
#define ZONE_TOTAL_LEN         ZONE_TOTAL_RX_LEN

#define PAYLOAD_LEN_TCP        1460
#define PAYLOAD_LEN_UDP        1024

#define TOE_HIJACK_MEMLEN      PAGE_SIZE
#define TOE_HIJACK_IP_OFFSET   512

#define min(a, b) ((a) > (b) ? (b) : (a))

/* type definition */
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef short int16_t;
//typedef char int8_t;
#define __be32 uint32_t
#define __be16 uint16_t
#define __be8  uint8_t
#define __le64 unsigned long long
#define dma_addr_t unsigned long
#define phys_addr_t unsigned long

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define _ALIGN_DOWN(addr,size)    ((addr)&(~((size)-1)))
#define PAGE_SIZE                 (1<<12)

#define TX_RECHECK_TIMES           3

struct pthread_event {
	int signaled;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

typedef struct {
	volatile int counter;
} atomic_t;

#define ATOMIC_INIT(i) {i}

#define atomic_read(v)		((v)->counter)
#define atomic_set(v,i)		(((v)->counter) = (i))

#define LOCK "lock ; "

static __inline__ void atomic_add(int i, atomic_t * v)
{
	__asm__ __volatile__(LOCK "addl %1,%0":"=m"(v->counter)
						 :"ir"(i), "m"(v->counter));
}

static __inline__ void atomic_sub(int i, atomic_t * v)
{
	__asm__ __volatile__(LOCK "subl %1,%0":"=m"(v->counter)
						 :"ir"(i), "m"(v->counter));
}

static __inline__ void atomic_inc(atomic_t * v)
{
	__asm__ __volatile__(LOCK "incl %0":"=m"(v->counter)
						 :"m"(v->counter));
}

static __inline__ void atomic_dec(atomic_t * v)
{
	__asm__ __volatile__(LOCK "decl %0":"=m"(v->counter)
						 :"m"(v->counter));
}

#define PTHREAD_EVENT_INIT(ev) do {             \
        pthread_event_init(&(ev));              \
    } while(0)

#define PTHREAD_EVENT_SET(ev) do {              \
        pthread_event_set(&(ev));               \
    } while(0)

#define PTHREAD_EVENT_WAIT(ev) do {             \
        pthread_event_wait(&(ev), -1);          \
    } while(0)

#define PTHREAD_EVENT_WAIT2(ev, timeout) do {       \
        int rc;                                     \
        errno = pthread_event_wait(&(ev), timeout); \
        rc = errno ? 0 : 1;                         \
    } while(0)

#define PTHREAD_EVENT_DESTROY(ev) do {          \
        pthread_event_destroy(&(ev));           \
    } while(0)

struct poll_addr {
	void *addr;
	phys_addr_t addr_phy;   /*物理地址*/
};

enum
{
	CONF_MKT_PRICE1 = 0,
	CONF_MKT_VOL1,
	CONF_TRADE_DAY,
	CONF_MULTICAST,
};

struct trade_day {
	int32_t low32;
	int32_t mid32;
	int8_t  high8;
};

struct toe_ip {
	uint8_t  toe_id;
	uint8_t  pad[3];
	uint32_t ip;
};

struct toe_mac {
	uint8_t toe_id;
	uint8_t pad;
	uint16_t up2;
	uint32_t low4;
};

struct multicast {
	uint8_t toe_id;
	uint8_t session_id;
	__be32 mcast_ip;
};

struct quote_config {
	int type;   
	union {
		u_int64_t mkt_price1;
		u_int32_t mkt_vol1;
		struct trade_day day;
		struct multicast mcast;
	} data;
};

enum l4_protocol {
	E_TCP = 0,
	E_UDP,
};

struct session_mem_mgr {
	uint8_t status;				/*0 :down;  1:up */
	uint8_t cpu_id;
	uint8_t toe_id;
	uint8_t session_id;
	volatile uint16_t tx_read_idx;
	volatile uint16_t tx_send_idx;
	volatile uint16_t tx_write_idx;
	volatile uint16_t rx_read_idx;
	volatile uint16_t rx_write_idx;

	void *rx_desc;
	void *tx_desc;
	dma_addr_t dma_rx_addr;
	dma_addr_t dma_tx_addr;
	__le64 rx_bus_addr;
	__le64 tx_bus_addr;
};

struct rx_head {
	uint32_t _rx_frame_cnt;
	uint16_t _rx_data_len;
	uint8_t _rx_link_id;
	uint8_t _toe_rx_err:1;
	uint8_t _toe_rx_udp:1;
	uint8_t _toe_rx_bypass:1;
	uint8_t __pad[24];
};

struct toe_rx {
	struct rx_head r_head;
#define r_frame_cnt  r_head._rx_frame_cnt
#define r_data_len   r_head._rx_data_len
#define r_link_id    r_head._rx_link_id
#define rx_err       r_head._toe_rx_err
#define rx_udp       r_head._toe_rx_udp
#define rx_bypass    r_head._toe_rx_bypass

	uint8_t data[ZONE_ENTRY_LEN - sizeof(struct rx_head)];
};

struct tx_head {
	int16_t _tx_addr;
	int16_t _tx_data_len;
	int8_t _tx_link_id;
	int8_t _tx_cmd;
	int32_t _tx_cmd_cnt;
	int32_t _padding[5];
};

struct toe_tx {
	struct tx_head t_head;
#define t_addr     t_head._tx_addr
#define t_data_len t_head._tx_data_len
#define t_link_id  t_head._tx_link_id
#define t_cmd      t_head._tx_cmd
#define t_cmd_cnt  t_head._tx_cmd_cnt

	uint8_t data[ZONE_ENTRY_LEN - sizeof(struct tx_head)];
};

struct session_init_struct {
	struct args_in {
		uint8_t cpu_id;
		uint8_t toe_id;
		uint8_t pro_type;		/*0:tcp; 1:udp */
		__be16 local_port;
		__be16 remote_port;
		__be32 remote_ip;
		__be32 local_ip;
	} in;
#define in_cpuid	in.cpu_id
#define in_toeid	in.toe_id
#define	in_protype	in.pro_type
#define	in_lport	in.local_port
#define in_rport	in.remote_port
#define in_lip		in.local_ip
#define	in_rip		in.remote_ip
	struct args_out {
		void *rx_addr;
		void *tx_addr;
		void *mgr_addr;
		phys_addr_t rx_addr_phy;
		phys_addr_t tx_addr_phy;
		phys_addr_t mgr_addr_phy;
		uint8_t session_id;
		uint8_t status;
	} out;
#define	out_raddr	out.rx_addr
#define	out_taddr	out.tx_addr
#define	out_maddr	out.mgr_addr
#define	out_praddr	out.rx_addr_phy
#define	out_ptaddr	out.tx_addr_phy
#define	out_pmaddr	out.mgr_addr_phy
#define	out_sid		out.session_id
#define out_stat    out.status

	void *mmap_mgr;
	void *mmap_rx;
	void *mmap_tx;
};

struct estb_session_struct {
	struct session_init_struct *session;
	unsigned long rbytes;
	unsigned long rpkts;
    unsigned long rlost;
	int8_t  wait;
	int8_t  destroy;
	uint8_t prev;
	uint8_t next;
};

typedef struct toe_tid_sid_fd_pid {
	uint8_t tid;
	uint8_t sid;
	uint8_t rsv[2];
	int fd;
	int pid;
}toe_tsfp_t;

struct toe_error {
	uint8_t  sid;
	uint8_t  tid;
	uint16_t r_idx;
	uint16_t w_idx;
	uint16_t r_pos;
	unsigned int  b_cnt;
	unsigned int  c_cnt;
	unsigned int  p_cnt;
	unsigned long rpkts;
    unsigned long rlost;
};

struct toe_overflow {
	uint8_t  sid;
	uint8_t  tid;
	uint16_t r_idx;
	uint16_t w_idx;
	unsigned long rlost;
	unsigned long rpkts;
};

extern void pthread_event_init(struct pthread_event *ev);
extern void pthread_event_destroy(struct pthread_event *ev);
extern void pthread_event_set(struct pthread_event *ev);
extern int pthread_event_wait(struct pthread_event *ev, unsigned long msecs);

/// api
struct session_init_struct *toe_connect(uint8_t cpu_id, uint8_t toe_id,
										uint8_t pro_type,
										unsigned short local_port,
										unsigned short remote_port,
										unsigned int local_ip, unsigned int remote_ip,
										int hijack_fd);
int toe_close(struct session_init_struct *session);
int toe_recv(struct session_init_struct *session, void *buf, unsigned long size);
int toe_send(struct session_init_struct *session, void *payload, uint32_t len);

int toe_setaffinity(int cpu);
void toe_show_established();
void *addr_mmap(unsigned long phy_addr, unsigned int mmap_len);

int toe_latency_test();
unsigned long toe_hijack_phyaddr();

int toe_quote_config(struct quote_config *cfg);
int toe_set_ip(char *ip, uint8_t toe_id);
int toe_set_mac(char *mac, uint8_t toe_id);

int toe_init();

#ifdef __cplusplus
}
#endif

#endif /* __TOE_APP_H_ */
