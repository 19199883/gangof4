#pragma once

#include <signal.h>     /* signal */
#include <log4cxx/logger.h>

using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

/*
SIGHUP        1       Term    Hangup detected on controlling terminal
or death of controlling process
*/
static void
SIGHUP_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"SIGHUP_handler was invoked.");
}



/*
SIGKILL       9       Term    Kill signal
*/
static void
SIGKILL_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"SIGKILL_handler was invoked.");
}


/*
SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
*/
static void
SIGPIPE_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"SIGPIPE_handler was invoked.");
}

/*
SIGALRM      14       Term    Timer signal from alarm(2)*/
static void
SIGALRM_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"SIGALRM_handler was invoked.");
}

/*
SIGTERM      15       Term    Termination signal
*/
static void
SIGTERM_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"SIGTERM_handler was invoked.");
}

/*
SIGUSR1   30,10,16    Term    User-defined signal 1
*/
static void
SIGUSR1_handler(int s)
{
//	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"SIGUSR1_handler was invoked.");
}

/*
SIGUSR2   31,12,17    Term    User-defined signal 2
*/
static void
SIGUSR2_handler(int s)
{
//	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGUSR2_handler was invoked.");
}

/*
SIGSTOP   17,19,23    Stop    Stop process
*/
static void
SIGSTOP_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGSTOP_handler was invoked.");
}

/*
SIGTSTP   18,20,24    Stop    Stop typed at tty
*/
static void
SIGTSTP_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGTSTP_handler was invoked.");
}

/*
SIGTTIN   21,21,26    Stop    tty input for background process
*/
static void
SIGTTIN_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGTTIN_handler was invoked.");
}

/*
SIGTTOU   22,22,27    Stop    tty output for background process
*/
static void
SIGTTOU_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGTTOU_handler was invoked.");
}

/*
SIGPOLL                 Term    Pollable event (Sys V). Synonym of SIGIO
*/
static void
SIGPOLL_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGPOLL_handler was invoked.");
}

/*
SIGPROF     27,27,29    Term    Profiling timer expired
*/
static void
SIGPROF_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGPROF_handler was invoked.");
}

/*
SIGVTALRM   26,26,28    Term    Virtual alarm clock (4.2BSD)
*/
static void
SIGVTALRM_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGVTALRM_handler was invoked.");
}

/*
SIGEMT       7,-,7      Term
*/
static void
SIGEMT_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGEMT_handler was invoked.");
}

/*
SIGSTKFLT    -,16,-     Term    Stack fault on coprocessor (unused)
*/
static void
SIGSTKFLT_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGSTKFLT_handler was invoked.");
}

/*
SIGIO       23,29,22    Term    I/O now possible (4.2BSD)
*/
static void
SIGIO_handler(int s)
{
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(), "SIGIO_handler was invoked.");
}

static void install(struct sigaction &act,int sig)
{	
	switch (sig){
	case SIGHUP:
		act.sa_handler = SIGHUP_handler;
		break;
	case SIGKILL:
		act.sa_handler = SIGKILL_handler;
		break;
	case SIGPIPE:
		act.sa_handler = SIGPIPE_handler;
		break;
	case SIGALRM:
		act.sa_handler = SIGALRM_handler;
		break;
	case SIGTERM:
		act.sa_handler = SIGTERM_handler;
		break;
	case SIGSTOP:
		act.sa_handler = SIGSTOP_handler;
		break;
	case SIGTSTP:
		act.sa_handler = SIGTSTP_handler;
		break;
	case SIGTTIN:
		act.sa_handler = SIGTTIN_handler;
		break;
	case SIGTTOU:
		act.sa_handler = SIGTTOU_handler;
		break;
//	case SIGPOLL:
//		act.sa_handler = SIGPOLL_handler;
//		break;
	case SIGPROF:
		act.sa_handler = SIGPROF_handler;
		break;
	case SIGVTALRM:
		act.sa_handler = SIGVTALRM_handler;
		break;
//	case SIGEMT:
//		act.sa_handler = SIGEMT_handler;
//		break;
	case SIGSTKFLT:
		act.sa_handler = SIGSTKFLT_handler;
		break;
	case SIGIO:
		act.sa_handler = SIGIO_handler;
		break;

	case SIGUSR1:
		act.sa_handler = SIGUSR1_handler;
		break;

	}
	
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(sig, &act, NULL);
}

static void install_sig_handlers()
{
	// SIGHUP        1       Term    Hangup detected on controlling terminal or death of controlling process
	struct sigaction SIGHUP_act;
	install(SIGHUP_act,SIGHUP);	

	// SIGINT        2       Term    Interrupt from keyboard
	struct sigaction SIGINT_act;
	install(SIGINT_act, SIGINT);

	// SIGKILL       9       Term    Kill signal
	struct sigaction SIGKILL_act;
	install(SIGKILL_act, SIGKILL);

	// SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
	struct sigaction SIGPIPE_act;
	install(SIGPIPE_act, SIGPIPE);

	// SIGALRM      14       Term    Timer signal from alarm(2)
	struct sigaction SIGALRM_act;
	install(SIGALRM_act, SIGALRM);

	// SIGTERM      15       Term    Termination signal
	struct sigaction SIGTERM_act;
	install(SIGTERM_act, SIGTERM);

	// SIGUSR1   30,10,16    Term    User-defined signal 1
	struct sigaction SIGUSR1_act;
	install(SIGUSR1_act, SIGUSR1);

	// SIGUSR2   31,12,17    Term    User-defined signal 2
//	struct sigaction SIGUSR2_act;
//	install(SIGUSR2_act, SIGUSR2);

	// SIGSTOP   17,19,23    Stop    Stop process
	struct sigaction SIGSTOP_act;
	install(SIGSTOP_act, SIGSTOP);

	// SIGTSTP   18,20,24    Stop    Stop typed at tty
	struct sigaction SIGTSTP_act;
	install(SIGTSTP_act, SIGTSTP);

	// SIGTTIN   21,21,26    Stop    tty input for background process
	struct sigaction SIGTTIN_act;
	install(SIGTTIN_act, SIGTTIN);

	// SIGTTOU   22,22,27    Stop    tty output for background process
	struct sigaction SIGTTOU_act;
	install(SIGTTOU_act, SIGTTOU);

	// SIGPOLL                 Term    Pollable event (Sys V). Synonym of SIGIO
//	struct sigaction SIGPOLL_act;
//	install(SIGPOLL_act, SIGPOLL);

	// SIGPROF     27,27,29    Term    Profiling timer expired
	struct sigaction SIGPROF_act;
	install(SIGPROF_act, SIGPROF);

	// SIGVTALRM   26,26,28    Term    Virtual alarm clock (4.2BSD)
	struct sigaction SIGVTALRM_act;
	install(SIGVTALRM_act, SIGVTALRM);

	// SIGEMT       7,-,7      Term
//	struct sigaction SIGEMT_act;
//	install(SIGEMT_act, SIGEMT);

	// SIGSTKFLT    -,16,-     Term    Stack fault on coprocessor (unused)
	struct sigaction SIGSTKFLT_act;
	install(SIGSTKFLT_act, SIGSTKFLT);

	// SIGIO       23,29,22    Term    I/O now possible (4.2BSD)
	struct sigaction SIGIO_act;
	install(SIGIO_act, SIGIO);
}
