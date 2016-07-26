#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <signal.h>

#include "toe_app.h"

#ifdef __cplusplus
extern "C" {
#endif	

static int toe_fd = -1;

static atomic_t inited = ATOMIC_INIT(0);
static atomic_t first = ATOMIC_INIT(-1);
static atomic_t curr = ATOMIC_INIT(-1);

#ifdef __SEMAPHORE__
static sem_t sem[MAX_SESSION_NUM*MAX_TOE_NUM];
#else
static struct pthread_event event[MAX_SESSION_NUM*MAX_TOE_NUM];
#endif

static pthread_mutex_t toe_connect_lock;
    
static pthread_t thread_poll;
static struct pthread_event ev_poll;
static int exit_poll = 0;

static struct estb_session_struct estb_session[MAX_SESSION_NUM*MAX_TOE_NUM];

static struct toe_error debug_err[256];
static struct toe_overflow info_of[256];

static int rx_threshhold = ZONE_ENTRY_RX_NUM/2;

static void *rx_addr_base[MAX_TOE_NUM];
static void *tx_addr_base[MAX_TOE_NUM];
static void *mgr_addr_base[MAX_TOE_NUM];

/*
            4KB hijack memory format
    +---------------------------------------+
    |        512B toe0 dst ip list          |
    +---------------------------------------+
    |        512B toe1 dst ip list          |
    +---------------------------------------+
    |                                       |
    |                                       |
    |            3KB fd/pid/sid             |
    |                                       |
    |                                       |
    |                                       |
    +---------------------------------------+
*/
void *hijack_addr;
    
/* 3KByte association with toe_id session_id fd pid */
toe_tsfp_t *krn_tsfp;

void pthread_event_init(struct pthread_event *ev)
{
	ev->signaled = 0;
	pthread_mutex_init(&(ev->mutex), 0);
	pthread_cond_init(&(ev->cond), 0);
}

void pthread_event_destroy(struct pthread_event *ev)
{
	if (EBUSY == pthread_cond_destroy(&ev->cond)) {
		pthread_mutex_lock(&ev->mutex);
		pthread_cond_broadcast(&ev->cond);
		pthread_mutex_unlock(&ev->mutex);
	}
	pthread_mutex_destroy(&ev->mutex);

	ev->signaled = 0;
}

void pthread_event_set(struct pthread_event *ev)
{
	pthread_mutex_lock(&ev->mutex);
	if (0 == ev->signaled) {
		ev->signaled = 1;
		pthread_cond_signal(&ev->cond);
	}
	pthread_mutex_unlock(&ev->mutex);
}

int pthread_event_wait(struct pthread_event *ev, unsigned long msecs)
{
	int ret = 0;
	struct timespec timeout;
	struct timeval now;

	if (pthread_mutex_lock(&ev->mutex) != 0) {
		return -1;
	}

	if (0 == ev->signaled) {
		if (msecs == -1) {
			ret = pthread_cond_wait(&ev->cond, &ev->mutex);
		} else {
			gettimeofday(&now, 0);
			timeout.tv_sec = now.tv_sec + msecs / 1000;
			timeout.tv_nsec = now.tv_usec * 1000 + msecs % 1000 * 1000000;
			if (timeout.tv_nsec >= 1000000000) {
				timeout.tv_nsec -= 1000000000;
				timeout.tv_sec++;
			}
			ret = pthread_cond_timedwait(&ev->cond, &ev->mutex, &timeout);
		}
	}

	ev->signaled = 0;

	pthread_mutex_unlock(&ev->mutex);
	return ret;
}

void *addr_mmap(unsigned long phy_addr, unsigned int mmap_len)
{
	int fd;
	unsigned long offset;
	void *mem_base = NULL;

	if (-1 == (fd = open("/dev/mem", O_RDWR)))
		return NULL;

	offset = phy_addr - _ALIGN_DOWN(phy_addr, PAGE_SIZE);
	mem_base = mmap(0, mmap_len,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd,
			_ALIGN_DOWN(phy_addr, PAGE_SIZE));
	if (MAP_FAILED == mem_base) {
		printf("mmap phy addr : 0x%x failed : %s\n", phy_addr, strerror(errno));
		close(fd);
		return NULL;
	}

	close(fd);

	return (void *)((char *)mem_base + offset);
}

unsigned long toe_hijack_phyaddr()
{
	struct poll_addr addr;
	
	if (ioctl(toe_fd, IOCTL_POLL, &addr)) {
		printf("IOCTL_POLL failed\n");
		return 0;
	}
	
	return addr.addr_phy;
}

int toe_latency_test()
{
	int opt = 1;
	return ioctl(toe_fd, IOCTL_LATENCY, &opt);
}

int toe_quote_config(struct quote_config *cfg)
{
	int fd;
	
	if (-1 == (fd = open(DRV_MYFPGA, O_RDWR))) {
		printf("can't open file %s\n", DRV_MYFPGA);
		return -1;
	}
	
	if (ioctl(fd, IOCTL_CONFIG, cfg)) {
		printf("IOCTL_POLL failed\n");
		return -1;
	}
	
	close(fd);
	
	return 0;
}

int toe_setaffinity(int cpu)
{
	/* sched_setaffinity */
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(cpu, &cpu_set);		// hardcode, bind to cpu3
	if (0 > sched_setaffinity(0, sizeof(cpu_set), &cpu_set)) {
		printf("bind thread to cpu%d failed:%s\n", cpu, strerror(errno));
		return -1;
	} else {
		printf("bind thread to cpu%d success!\n", cpu);
	}

	return 0;
}

void toe_show_established()
{
	int i = 0;
	
	uint16_t rmask = ZONE_ENTRY_RX_NUM - 1;
	
	printf("TOE API Version : %s - %s\n", __DATE__, __TIME__);
	
	for (i = 0; i < MAX_SESSION_NUM*MAX_TOE_NUM; i++) {
		if (estb_session[i].session) {
			struct session_mem_mgr *s_mgr =	(struct session_mem_mgr *)estb_session[i].session->mmap_mgr;
			printf("\r\n---- session : %d\n", i);
			printf("     -> struct session_mem_mgr info : \n");
			printf("                    status : %u\n", s_mgr->status);
			printf("                    cpu_id : %u\n", s_mgr->cpu_id);
			printf("                    toe_id : %u\n", s_mgr->toe_id);
			printf("                session_id : %u\n", s_mgr->session_id);
			printf("               tx_read_idx : %u\n", s_mgr->tx_read_idx);
			printf("               tx_send_idx : %u\n", s_mgr->tx_send_idx);
			printf("              tx_write_idx : %u\n", s_mgr->tx_write_idx);
			printf("               rx_read_idx : %u\n", s_mgr->rx_read_idx);
			printf("              rx_write_idx : %u\n", s_mgr->rx_write_idx);
			printf("               dma_rx_addr : 0x%x\n", s_mgr->dma_rx_addr);
			printf("               dma_tx_addr : 0x%x\n", s_mgr->dma_tx_addr);
			printf("     -> struct estb_session info : \n");
			printf("                      prev : %u\n", estb_session[i].prev);
			printf("                      next : %u\n", estb_session[i].next);
			printf("                    rbytes : %lu\n", estb_session[i].rbytes);
			printf("                     rpkts : %lu\n", estb_session[i].rpkts);
			printf("                     rlost : %lu\n", estb_session[i].rlost);
			printf("\n\n");
		}
	}
	
	printf("\r\n---- overflow :\n");
	for (i = 0; i < 256; i++) {
		if (0xff != info_of[i].sid) {
			printf("\r\n     -> %d\n", i);
			printf("        sid : %u\n", info_of[i].sid);
			printf("        tid : %u\n", info_of[i].tid);
			printf("      r_idx : %u\n", info_of[i].r_idx);
			printf("      w_idx : %u\n", info_of[i].w_idx);
			printf(" last rlost : %lu\n", info_of[i].rlost);
			printf(" last rpkts : %lu\n", info_of[i].rpkts);
		}
	}
	
	printf("\r\n---- error :\n");
	for (i = 0; i < 256; i++) {
		if (0xff != debug_err[i].sid) {
			printf("\r\n     -> %d\n", i);
			printf("        sid : %u\n", debug_err[i].sid);
			printf("        tid : %u\n", debug_err[i].tid);
			printf("      r_idx : %u(%u)\n", debug_err[i].r_idx, debug_err[i].r_idx&rmask);
			printf("      w_idx : %u(%u)\n", debug_err[i].w_idx, debug_err[i].w_idx&rmask);
			printf("      r_pos : %u\n", debug_err[i].r_pos);
			printf("      b_cnt : %lu\n", debug_err[i].b_cnt);
			printf("      c_cnt : %lu\n", debug_err[i].c_cnt);
			printf("      p_cnt : %lu\n", debug_err[i].p_cnt);
			printf(" last rlost : %lu\n", debug_err[i].rlost);
			printf(" last rpkts : %lu\n", debug_err[i].rpkts);
		}
	}
}

static void toe_free_session(struct session_init_struct *session)
{
	uint8_t toe_id;
	uint8_t sid;
	uint8_t index;
	uint8_t prev;
	uint8_t next;
		
	if (!session)
		return ;
	
	toe_id = session->in_toeid;
	sid = session->out_sid;
	index = toe_id*MAX_SESSION_NUM + sid;
	prev = estb_session[index].prev;
	next = estb_session[index].next;
	
	/* update first, curr */
	if (index == (uint8_t)atomic_read(&first))
		atomic_set(&first, next);

	if (index == (uint8_t)atomic_read(&curr)) {
		if (0xff != next)
			atomic_set(&curr, next);
		else if (0xff != prev)
			atomic_set(&curr, prev);
		else
			atomic_set(&curr, 0xff);
	}	

	/* delete from estb_session */
	estb_session[prev].next = next;
	estb_session[next].prev = prev;	
	
#ifdef __SEMAPHORE__
	sem_destroy(&sem[index]);
#else
	PTHREAD_EVENT_DESTROY(event[index]);
#endif

	free(session);
	session = NULL;
}

static void *toe_data_poll(void *param)
{
	struct session_mem_mgr *s_mgr;
	uint8_t index, next;
	uint8_t toe_id, sid;

	PTHREAD_EVENT_SET(ev_poll);

	toe_setaffinity(9);

	/* polling data */
	while (!exit_poll) {
		index = (uint8_t)atomic_read(&first);
		while (0xff != index && estb_session[index].session && estb_session[index].session->mmap_mgr) {
			next = estb_session[index].next;
			if (estb_session[index].destroy) {
				toe_free_session(estb_session[index].session);
				break;
			}
			
			s_mgr = (struct session_mem_mgr *)estb_session[index].session->mmap_mgr;
			if (s_mgr->rx_read_idx != s_mgr->rx_write_idx) {
				if (1 & estb_session[index].wait) {
#ifdef __SEMAPHORE__
					sem_post(&sem[index]);
#else
					PTHREAD_EVENT_SET(event[index]);
#endif
				}
			}
			index = next;
		}

		/* yield cpu? */
	}

	return 0;
}

static int toe_session_init(uint8_t index)
{
	pthread_attr_t attr;
		
	estb_session[index].session = NULL;
	estb_session[index].prev = 0xff;
	estb_session[index].next = 0xff;
	estb_session[index].rbytes = 0;
	estb_session[index].rpkts = 0;
    estb_session[index].rlost = 0;
	estb_session[index].wait = 1;
	estb_session[index].destroy = 0;
#ifdef __SEMAPHORE__
	sem_init(&sem[index], 0, 0);
#else
	PTHREAD_EVENT_INIT(event[index]);
#endif

	if (atomic_read(&inited))
		return 0;
	
	PTHREAD_EVENT_INIT(ev_poll);

	/* create a thread for polling data */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	pthread_create(&thread_poll, &attr, toe_data_poll, NULL);
	PTHREAD_EVENT_WAIT(ev_poll);	
	
	atomic_set(&inited, 1);
	
	return 0;
}

struct session_init_struct *toe_connect(uint8_t cpu_id, uint8_t toe_id,
		uint8_t pro_type,
		unsigned short local_port,
		unsigned short remote_port,
		unsigned int local_ip, unsigned int remote_ip,
		int hijack_fd)
{
	struct session_init_struct *session = NULL;
	struct session_mem_mgr *s_mgr = NULL;
	uint8_t sid, index, cidx, c_next_idx;

	session = (struct session_init_struct *)malloc(sizeof(struct session_init_struct));
	if (NULL == session) {
		return NULL;
	}

	memset(session, 0, sizeof(struct session_init_struct));
	session->in_cpuid = cpu_id;
	session->in_toeid = toe_id;
	session->in_protype = pro_type;
	session->in_lport = local_port;
	session->in_rport = remote_port;
	session->in_lip = htonl(local_ip);
	session->in_rip = htonl(remote_ip);
	session->mmap_mgr = NULL;
	session->mmap_rx = NULL;
	session->mmap_tx = NULL;

    pthread_mutex_lock(&toe_connect_lock);

	if (-1 == ioctl(toe_fd, IOCTL_CONNECT, session)) {
		free(session);
		session = NULL;
		goto out;
	}

	if (CONNECTING != session->out_stat) {
		printf("connect failed : %d\n", session->out_stat);
		free(session);
		session = NULL;
		goto out;
	}
	
	sid = session->out_sid;

	session->mmap_mgr = mgr_addr_base[toe_id] + sid*sizeof(struct session_mem_mgr);
	session->mmap_rx  = rx_addr_base[toe_id] + sid*ZONE_TOTAL_RX_LEN;
	session->mmap_tx  = tx_addr_base[toe_id] + sid*ZONE_TOTAL_TX_LEN;

    /* pid must be set in my hijack mem for toe driver in kernelspace */
    krn_tsfp[sid+toe_id*MAX_SESSION_NUM].pid = getpid();

	s_mgr = (struct session_mem_mgr *)session->mmap_mgr;

	// wait for driver set status in manage memory
	sleep(1);

	if (1 == s_mgr->status) {
		index = sid+toe_id*MAX_SESSION_NUM;	
		toe_session_init(index);

        if (0xff == (uint8_t)atomic_read(&first))
			atomic_set(&first, index);

        /* add this session into established sessions list */
		cidx = (uint8_t)atomic_read(&curr);

		estb_session[index].prev = cidx;
		estb_session[index].next = 0xff;
		estb_session[index].session = session;

		if (0xff != cidx) {
			c_next_idx = estb_session[cidx].next;
			estb_session[cidx].next = index;
			estb_session[c_next_idx].prev = index;
			estb_session[index].next = c_next_idx;
		}

        atomic_set(&curr, index);

        /* set sid tid in kernelspace */
        krn_tsfp[sid+toe_id*MAX_SESSION_NUM].sid = sid;
        krn_tsfp[sid+toe_id*MAX_SESSION_NUM].tid = toe_id;

        if (-1 != hijack_fd) {
            krn_tsfp[sid+toe_id*MAX_SESSION_NUM].fd  = hijack_fd;
            ioctl(toe_fd, IOCTL_HIJACK, session);
        }

		printf("connect success info --> cpu_id:%u, toe_id:%u, pro_type:%u, local_port:%u, remote_port:%u, local_ip:0x%x, remote_ip:0x%x\n",
				cpu_id, toe_id, pro_type, local_port, remote_port,
				local_ip, remote_ip);
	} else {
		printf("connect fail info [0x%x] --> cpu_id:%u, toe_id:%u, pro_type:%u, local_port:%u, remote_port:%u, local_ip:0x%x, remote_ip:0x%x\n",
				s_mgr->status, cpu_id, toe_id, pro_type, local_port, remote_port,
				local_ip, remote_ip);
		free(session);
		session = NULL;
		
	}

out:
    pthread_mutex_unlock(&toe_connect_lock);

	return session;
}

int toe_close(struct session_init_struct *session)
{
	uint8_t toe_id = session->in_toeid;
	uint8_t sid = session->out_sid;
	uint8_t index = sid + toe_id*MAX_SESSION_NUM;

	struct session_mem_mgr *s_mgr = NULL;

    if (!session)
        return 0;
    
    s_mgr = (struct session_mem_mgr *)session->mmap_mgr;

	if (estb_session[index].destroy) {
		printf("sid:%u, tid:%u call toe_close more times, error happens\n", sid, toe_id);
		return 0;
	}
	
	if (-1 == toe_fd) {
		printf("toe_fd was closed, error happens\n");
		return -1;
	}

	if (-1 == ioctl(toe_fd, IOCTL_CLOSE, session)) {
		printf("ioctl IOCTL_CLOSE fail, sid:%u, tid:%u\n", sid, toe_id);
		return -1;
	}
	
	estb_session[index].destroy = 1;
	
	printf("toe_close success, sid:%u, tid:%u\n", sid, toe_id);

	return 0;
}

int __toe_recv(struct session_init_struct *session, char *buf, long size)
{
	uint8_t toe_id = session->in_toeid;
	uint8_t sid = session->out_sid;
	uint8_t index = sid + toe_id*MAX_SESSION_NUM;
	
	static uint8_t err = 0;
	static uint8_t of  = 0;

	unsigned long r_bytes = 0;
	uint16_t r_pkts  = 0;

	struct session_mem_mgr *s_mgr = NULL;
	struct toe_rx *rx_base, *rx;

	uint16_t mask = ZONE_ENTRY_RX_NUM - 1;
	
	uint16_t pkts = 0;
	
	uint32_t cur_cnt = 0;
	uint32_t base_cnt = 0;
	uint32_t pre_cnt = 0;

	rx_base = (struct toe_rx *)session->mmap_rx;
	s_mgr = (struct session_mem_mgr *)session->mmap_mgr;

	uint16_t r_idx = s_mgr->rx_read_idx;
	uint16_t w_idx = s_mgr->rx_write_idx;

    /* read zone is empty */
	if (!(r_idx ^ w_idx)) {
		errno = EAGAIN;
		if (1 ^ s_mgr->status)
			errno = ENOTCONN;
		return -1;
	}
	
	pkts = w_idx - r_idx;

    /* if pkts > ZONE_ENTRY_RX_NUM, recieve pkts = ZONE_ENTRY_RX_NUM(or ZONE_ENTRY_RX_NUM-32 for recieve zone head) */
    if (pkts > ZONE_ENTRY_RX_NUM) {
		info_of[of].sid   = sid;
		info_of[of].tid   = toe_id;
		info_of[of].r_idx = r_idx;
		info_of[of].w_idx = w_idx;
		info_of[of].rlost = estb_session[index].rlost;
		info_of[of].rpkts = estb_session[index].rpkts;
		of++;

#if 0		
        estb_session[index].rlost += pkts - ZONE_ENTRY_RX_NUM + 32;
		r_idx = w_idx - ZONE_ENTRY_RX_NUM + 32;
		pkts = ZONE_ENTRY_RX_NUM - 32;
#else		
		estb_session[index].rlost += pkts - ZONE_ENTRY_RX_NUM;
		r_idx = w_idx - ZONE_ENTRY_RX_NUM;
		pkts = ZONE_ENTRY_RX_NUM;
#endif		
    }
	
	base_cnt = *(uint32_t *)rx_base;
	
	while (r_pkts < pkts) {
		rx = rx_base + ((r_idx+r_pkts)& mask);
		if ((r_bytes + rx->r_data_len) > size)
			break;
		
		cur_cnt = *(uint32_t *)rx;
		if (!r_pkts) {
			pre_cnt = cur_cnt-1;
		}
		
		if (cur_cnt != (pre_cnt+1)) {
			debug_err[err].sid = sid;
			debug_err[err].tid = toe_id;
			debug_err[err].r_idx = r_idx;
			debug_err[err].w_idx = w_idx;
			debug_err[err].r_pos = r_pkts;
			debug_err[err].b_cnt = base_cnt;
			debug_err[err].c_cnt = cur_cnt;
			debug_err[err].p_cnt = pre_cnt;
			debug_err[err].rlost = estb_session[index].rlost;
			debug_err[err].rpkts = estb_session[index].rpkts;
			err++;
		}
		
		pre_cnt = cur_cnt;

#if 0		
		memcpy(buf + r_bytes, rx->data-sizeof(struct rx_head), rx->r_data_len+sizeof(struct rx_head));
		r_bytes += rx->r_data_len+sizeof(struct rx_head);
#else
		memcpy(buf + r_bytes, rx->data, rx->r_data_len);
		r_bytes += rx->r_data_len;
#endif
	
		r_pkts++;

		if (!(r_pkts ^ rx_threshhold)) {
			/* return, give pkts to app */
			break;
		}
	}

	s_mgr->rx_read_idx = r_idx+r_pkts;

	estb_session[index].rbytes += r_bytes;
	estb_session[index].rpkts  += r_pkts;

	return r_bytes;
}

int toe_recv(struct session_init_struct *session, void *buf, unsigned long size)
{
	uint8_t toe_id = session->in_toeid;
	uint8_t sid = session->out_sid;
	uint8_t index = sid + toe_id*MAX_SESSION_NUM;

	if (1 & estb_session[index].destroy) {
		errno = EBADF;
		return -1;
	}

	if (1 & estb_session[index].wait) {
#ifdef __SEMAPHORE__
		sem_wait(&sem[index]);
#else
		PTHREAD_EVENT_WAIT(event[index]);
#endif
		estb_session[index].wait = 0;
	}

	return __toe_recv(session, (char *)buf, size);
}

/* as default, packet payload length < zone total length, index with tx isnot monotone increase */
int toe_send(struct session_init_struct *session, void *payload, uint32_t len)
{
	int redo = 0;
	uint8_t proto;
	uint16_t r_idx, w_idx;
	uint32_t i, j, k;
	uint8_t toe_id = session->in_toeid;
	uint8_t sid = session->out_sid;
	uint8_t index = sid + toe_id*MAX_SESSION_NUM;

	uint32_t zone_entrys;
	uint32_t zone_data_len;

	int w_bytes = 0;

	struct toe_tx *tx_base, *tx = NULL;
	struct session_mem_mgr *s_mgr = NULL;

	uint16_t mask = ZONE_ENTRY_TX_NUM - 1;
	
	uint16_t set_protol[2] = {0xa600, 0xa500};

	tx_base = (struct toe_tx *)session->mmap_tx;
	s_mgr = (struct session_mem_mgr *)session->mmap_mgr;

    /* if tx zone is full, wait TX_RECHECK_TIMES(:3) useconds for send data */
recheck:
	if (1 != s_mgr->status) {
		errno = ECONNRESET;
		return -1;
	}

	if (unlikely(TX_RECHECK_TIMES == redo)) {
		errno = ENOBUFS;
		return -1;
	}
	
	r_idx = s_mgr->tx_read_idx;
	w_idx = s_mgr->tx_write_idx;
	
	if (unlikely(((w_idx+1)&mask) == r_idx)) {
		usleep(1);
		redo++;
		goto recheck;
	}

	proto = session->in_protype;
	
	zone_data_len = (E_TCP == proto) ? PAYLOAD_LEN_TCP : PAYLOAD_LEN_UDP;

    /* send pkt buffer length <= tx zone total length */
	if (unlikely(len > zone_data_len*ZONE_ENTRY_TX_NUM))
		len = zone_data_len*ZONE_ENTRY_TX_NUM;

	j = len % zone_data_len;
	k = len / zone_data_len;
	
	zone_entrys = j ? (k+1) : k;

	if (r_idx != w_idx)
		zone_entrys = min((r_idx-w_idx)&mask, zone_entrys);
			
	for (i = 0; i < zone_entrys; i++) {
		uint32_t c_w_idx = ((w_idx + i) & mask);
		uint32_t n_w_idx = ((c_w_idx + 1) & mask);
		
		if (n_w_idx == r_idx)
			break;
		
		tx = tx_base + c_w_idx;

        /* set flags in tx head */
		/*
		if (E_TCP == proto)
			*((unsigned int *)tx + 1) = (n_w_idx << 16) | 0xa600 | (sid & 0xff);
		else
			*((unsigned int *)tx + 1) = (n_w_idx << 16) | 0xa500 | (sid & 0xff);
		*/
		
		*((unsigned int *)tx + 1) = (n_w_idx << 16) | set_protol[proto] | (sid & 0xff);

		if ((len-w_bytes) > zone_data_len)
			tx->t_data_len = zone_data_len;
		else
			tx->t_data_len = len-w_bytes;
		
		memcpy(tx->data, payload + w_bytes, tx->t_data_len);
		
		//toe_latency_test();
		
		w_bytes += tx->t_data_len;
		s_mgr->tx_write_idx = n_w_idx;
		
		// as default, driver is ok
		ioctl(toe_fd, IOCTL_SEND, session);
	}

	return w_bytes;
}

void toe_exit()
{
	uint8_t toe_id;
	unsigned long i = 0;
	
	for (i = 0; i < MAX_SESSION_NUM*MAX_TOE_NUM; i++)
		estb_session[i].destroy = 1;

	exit_poll = 1;
	if (atomic_read(&inited)) {
		while (!pthread_kill(thread_poll, 0))
			usleep(1000);
	}
	
	if (-1 != toe_fd)
		close(toe_fd);
	
	for (toe_id = 0; toe_id < MAX_TOE_NUM; toe_id++) {
		if (rx_addr_base[toe_id])
			munmap(rx_addr_base[toe_id], ZONE_TOTAL_RX_LEN*MAX_SESSION_NUM);
		
		if (tx_addr_base[toe_id])
			munmap(tx_addr_base[toe_id], ZONE_TOTAL_TX_LEN*MAX_SESSION_NUM);
		
		if (mgr_addr_base[toe_id])
			munmap(mgr_addr_base[toe_id], sizeof(struct session_mem_mgr)*MAX_SESSION_NUM);
	}
	
	if (hijack_addr)
		munmap(hijack_addr, TOE_HIJACK_MEMLEN);
	
	return ;
}

//__attribute__ ((constructor)) 
int toe_init()
{
	uint8_t toe_id;
	unsigned long hijack_phyaddr;
	struct session_init_struct init;
	char *env = NULL;
	
	if (-1 == (toe_fd = open(DRV_MYFPGA, O_RDONLY))) {
		printf("can't open %s : %s\n", DRV_MYFPGA, strerror(errno));
		return -1;
	}
	
	for (toe_id = 0; toe_id < MAX_TOE_NUM; toe_id++) {
		memset(&init, 0, sizeof(init));
		
		init.in_toeid = toe_id;
		if (-1 == ioctl(toe_fd, IOCTL_INIT, &init)) {
			printf("toe%u toe_init, IOCTL_INIT fail\n", toe_id);
			close(toe_fd);
			return -1;
		}
		
		rx_addr_base[toe_id]  = addr_mmap(init.out_praddr, ZONE_TOTAL_RX_LEN*MAX_SESSION_NUM);
		tx_addr_base[toe_id]  = addr_mmap(init.out_ptaddr, ZONE_TOTAL_TX_LEN*MAX_SESSION_NUM);
		mgr_addr_base[toe_id] = addr_mmap(init.out_pmaddr, sizeof(struct session_mem_mgr)*MAX_SESSION_NUM);
	}
	
	hijack_phyaddr = toe_hijack_phyaddr();
	/* hijack memory size is 1page = 4096Byte */
	hijack_addr    = addr_mmap(hijack_phyaddr, TOE_HIJACK_MEMLEN);
	
	/* 3KByte of association with toeid, sessionid, fd, pid. just only for 16 sessions in 2 toes */
	krn_tsfp = (toe_tsfp_t *)((char *)hijack_addr+TOE_HIJACK_IP_OFFSET*MAX_TOE_NUM);

	memset(debug_err, 0xff, sizeof(debug_err));
	memset(info_of, 0xff, sizeof(info_of));

	env = getenv("TOE_RCV_THRESHHOLD");
	if (env)
		rx_threshhold = atoi(env);

    pthread_mutex_init(&toe_connect_lock, 0);
	
	atexit(toe_exit);

	return 0;
}

#ifdef __cplusplus
}
#endif
