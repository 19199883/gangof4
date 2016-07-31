# Makefile 

CC:=g++
DEBUG:=n
TOP:=..
INCPATH:=-I$(TOP)/lib/include -I$(TOP)/lib/include/citics_hs -I$(TOP)/lib/include/my -I$(TOP)/my_common/include -I$(TOP)/my_compliance_checker/include -I$(TOP)/tunnel_common_src -I$(TOP)/include_tunnel
LIBPATH:=-L$(TOP)/lib/bin -L$(TOP)/lib/bin/citics_hs -L$(TOP)/lib/bin/my -L$(TOP)/my_common/bin -L$(TOP)/my_compliance_checker/bin
CPPFLAGS:= -std=c++11 $(INCPATH)
CFLAGS:= -fPIC -fvisibility=hidden
LDFLAGS:= -shared -Wl,--exclude-libs,ALL
OBJPATH:=./obj
BINPATH:=./bin
LIBS:= -lmxml -lpthread   -lssl -lmy_common_v2 -lmy_compliance_checker -lqtm -lmymarketdata -lCITICs_HsT2Hlp -lt2sdk64
DEPS:=.depends
OUT:=libcitics_hs_tunnel.so

SUBDIR:=./src ../tunnel_common_src

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
