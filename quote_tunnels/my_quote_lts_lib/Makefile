# Makefile 

CC:=g++
DEBUG:=n
CRACK:=y
TOP:=..
INCPATH:=-I$(TOP)/lib/include -I$(TOP)/lib/include/lts -I$(TOP)/lib/include/my -I$(TOP)/my_common/include -I$(TOP)/quote_common_src -I$(TOP)/include_quote/include_stock
LIBPATH:=-L$(TOP)/lib/bin -L$(TOP)/lib/bin/lts -L$(TOP)/lib/bin/my -L$(TOP)/my_common/bin
CFLAGS:= -fPIC -fvisibility=hidden
LDFLAGS:= -shared -Wl,--exclude-libs,ALL
OBJPATH:=./obj
BINPATH:=./bin
DEPS:=.depends

ifeq ($(strip $(CRACK)),y)
	CPPFLAGS:= -DUSE_LTS_API_2 -std=c++11 $(INCPATH)	
	LIBS:= -lmxml -lpthread -lmy_common_v2 -lqtm -lltsapi -lsecurityqueryapi -lsecuritymduserapi -lboost_system   -lboost_filesystem -lboost_date_time
	OUT:=libmy_quote_lts_opt_lib.so
else
	CPPFLAGS:= -std=c++11 $(INCPATH)	
	LIBS:= -lmxml -lpthread -lmy_common_v2 -lqtm -lsecurityqueryapi -lsecuritymduserapi -lboost_system   -lboost_filesystem -lboost_date_time
	OUT:=libmy_quote_lts_opt_sim_lib.so
endif

SUBDIR:=./src ../quote_common_src

vpath %.cpp $(SUBDIR)

ifeq ($(strip $(DEBUG)),y)
	CFLAGS+= -g3 -O0
	OUT:=$(addsuffix d, $(OUT))
else
	CFLAGS+=-O2
endif

SRCS:=$(foreach d, $(SUBDIR), $(wildcard $(d)/*.cpp))
OBJS:=$(patsubst %.cpp,%.o,$(SRCS))
OBJS:=$(addprefix $(OBJPATH)/, $(notdir $(OBJS)))
OUT:=$(addprefix $(BINPATH)/, $(OUT))


all:$(OUT)
	@echo $(OUT)	

$(OUT):$(DEPS) $(OBJS)
	-@mkdir -p $(BINPATH)
	$(CC) $(OBJS) 	-o $@  $(LDFLAGS) $(LIBPATH) $(LIBS)	
	@echo "---------build target finshed-----------"


$(OBJPATH)/%.o:%.cpp
	-@mkdir -p $(OBJPATH)
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH)  -o $@

$(DEPS):$(SRCS)
	-@rm $(DEPS)
	$(CC)  -MM $(CPPFLAGS) $(INCPATH) $^  >>$(DEPS) 

-include $(DEPS)

distclean:
	-@rm $(DEPS)
	-@rm $(OUT)
	-@rm $(OBJS)

clean:
	-@rm $(OUT)
	-@rm $(OBJS)

help:
	@echo "make (all):\t This is the default command when target unspecified "
	@echo "make clean:\t clean intermediate objects, target"
	@echo "make distclean:\t clean depends, intermediate objects, target"
