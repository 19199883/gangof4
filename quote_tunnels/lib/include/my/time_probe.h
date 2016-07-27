#ifndef TIME_PROBE_H
#define TIME_PROBE_H

#ifdef __cplusplus
extern "C"
{
#endif
	/*
	 * To enable time_prober, you should :
	 * 1. define macro 'PROBE_COST'
	 * 2. call TIME_PROBER_INIT when system start up.
	 * 3. insert TIME_PROBER_BEGIN & TIME_PROBER_END to your codes.
	 * 4. call TIME_PROBER_EXIT when system exit.
	 */

	void
		time_prober_init(void);

	void
		time_prober_set_tag_name(const int tag, const char* name);

	void
		time_prober_begin(const int tag);

	void
		time_prober_end(const int tag);

	void
		time_prober_exit(void);

#ifdef PROBE_COST
#define TIME_PROBER_INIT \
	do \
			{\
		time_prober_init();\
			} while (0);

#define TIME_PROBER_SET_TAG_NAME(tag, name)	\
	do \
		{\
		time_prober_set_tag_name(tag, name);	\
		} while (0);

#define TIME_PROBER_EXIT	\
	do\
		{\
		time_prober_exit();\
		} while (0);

#define TIME_PROBER_BEGIN(tag) \
	do \
		{	\
		time_prober_begin(tag);	\
		} while (0);

#define TIME_PROBER_END(tag) \
	do \
		{	\
		time_prober_end(tag);	\
		} while (0);

#else

#define TIME_PROBER_INIT
#define TIME_PROBER_EXIT
#define TIME_PROBER_BEGIN(tag)
#define TIME_PROBER_END(tag)
#define TIME_PROBER_SET_TAG_NAME(tag, name)

#endif


#ifdef __cplusplus
}
#endif

#endif