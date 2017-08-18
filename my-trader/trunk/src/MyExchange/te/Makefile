# Makefile 

CC:=g++
AR:=ar
DEBUG:=n
TIME_PROBER:=n
TOP:=../../../
INCPATH:= 

LIBPATH:=
CPPFLAGS:= -v -c  -std=c++11 $(INCPATH) -fmessage-length=0 
CFLAGS:= 

LDFLAGS:=  
OBJPATH:=./obj
BINPATH:=./bin
LIBS:=
DEPS:=.depends
OUT:=libte.a

SUBDIR:=./

vpath %.cpp $(SUBDIR)

ifeq ($(strip $(DEBUG)),y)
	CFLAGS+= -g3 -O0	
else
	CFLAGS+=-O3
endif

SRCS:=$(foreach d, $(SUBDIR), $(wildcard $(d)/*.cpp))
OBJS:=$(patsubst %.cpp,%.o,$(SRCS))
OBJS:=$(addprefix $(OBJPATH)/, $(notdir $(OBJS)))
OUT:=$(addprefix $(BINPATH)/, $(OUT))


all:$(OUT)
	@echo $(OUT)	

$(OUT):$(DEPS) $(OBJS)
	-@mkdir -p $(BINPATH)
	$(AR) r $@ $(OBJS) 	
	@echo "---------build target finshed-----------"


$(OBJPATH)/%.o:%.cpp
	-@mkdir -p $(OBJPATH)
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH)  -o $@

$(DEPS):$(SRCS)
	-@rm $(DEPS)
	$(CC)  -M $(CPPFLAGS) $(INCPATH) $^  >>$(DEPS) 

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
