#include "maint.h"
#include <log4cxx/logger.h>
#include <stdio.h>
#include <log4cxx/xml/domconfigurator.h>

using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

bool maintenance::enabled_ = false;

void maintenance::assemble()
{
	maintenance::disable();

	 struct sigaction oldact,act;
	 act.sa_sigaction=handle_sig;
	 act.sa_flags=SA_SIGINFO;//表示使用sa_sigaction指示的函数，处理完恢复默认，不阻塞处理过程中到达下在被处理的信号
	 //注册信号处理函数
     sigaction(SIGUSR2,&act,&oldact);

}

bool maintenance::enabled()
{
	return maintenance::enabled_;
}

void maintenance::log(string &content)
{
	if(maintenance::enabled_){
		LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),content); 
	}
}

void maintenance::handle_sig(int signo,siginfo_t *si,void *ucontext)
{
	if(maintenance::enabled_){
		maintenance::disable();
	}else{
		maintenance::enable();
	}
}

void maintenance::enable()
{
	maintenance::enabled_ = true;
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"enter maint. mode"); 
}

void maintenance::disable()
{
	maintenance::enabled_ = false;
	LOG4CXX_INFO(log4cxx::Logger::getRootLogger(),"exit maint. mode"); 
}

