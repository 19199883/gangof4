# Makefile 

CC:=g++
DEBUG:=n
TOP:=../../../
VERSION:=v2.6.x
DIST_DIR:=$(TOP)/dist
# to be changed
PROJECT_NAME:=quote_forwarder_shfe_jr

INCPATH:= -I$(TOP)/lib/tunnel/include\
		  -I$(TOP)/lib/quote/include \
		  -I$(TOP)/lib/ \
	  	  -I$(TOP)/lib/toe \
	  	  -I$(TOP)/lib/my_common \
	  	  -I../utils \
	  	  -I../tinyxml \
	  	  -I../my_exchange_entity \
	  	  -I../quote_forwarder_common \
	  	  -I../$(PROJECT_NAME)

LIBPATH:= -L$(TOP)/lib/toe \
      -L$(TOP)/lib/common \
      -L$(TOP)/lib/quote/shfe_my_jr_new/bin \
      -L../utils/bin \
	  -L../tinyxml/bin \
	  -L../my_exchange_entity/bin \
	  -L../quote_forwarder_common/bin \
	  -L../my_exchange_db/bin \
	  -L$(TOP)/lib/quote

CPPFLAGS:= -v -std=c++11 $(INCPATH)
CFLAGS:= 
LDFLAGS:= 
OBJPATH:=./obj
BINPATH:=./bin
LIBS:=  -lquote_forwarder_common -lqtm -ltinyxml -lboost_system \
		-lpthread -lboost_thread -llog4cxx -lmy_quote_shfe_my_jr_new_lib \
		-lmy_common_v2 -lthostmduserapi -lthosttraderapi

DEPS:=.depends

# bo be changed
OUT:=shfe_jr_mktsvc
PACKAGE_NAME:=$(OUT)

SUBDIR:=./
CURDIR:=.

vpath %.cpp $(SUBDIR)

ifeq ($(strip $(DEBUG)),y)
	CFLAGS+= -g3 -O0	
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
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	-@mkdir -p $(BINPATH)
	$(CC) $(LIBPATH) -o $@  $(OBJS) $(LDFLAGS) $(LIBS) 
	@echo "---------build target finshed-----------"


$(OBJPATH)/%.o:%.cpp
	-@mkdir -p $(OBJPATH)
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(INCPATH)  -o $@
	@echo Finished building: $<

$(DEPS):$(SRCS)
	-@rm $(DEPS)
	$(CC)  -M $(CPPFLAGS) $(INCPATH) $^  >>$(DEPS) 

-include $(DEPS)

dist:
	rm -fr $(DIST_DIR)/$(PACKAGE_NAME)
	mkdir -p $(DIST_DIR)/$(PACKAGE_NAME)	
	-cp -varf $(CURDIR)/bin/* $(DIST_DIR)/$(PACKAGE_NAME)/	
	-cp -varf $(CURDIR)/*.key $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -varf $(CURDIR)/*.xml $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -vaf $(TOP)/lib/quote/shfe_my_jr_new/bin/* $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -vaf $(TOP)/lib/quote/shfe_my_jr_new/config/* $(DIST_DIR)/$(PACKAGE_NAME)/	
	-cp -varf $(TOP)/lib/common/* $(DIST_DIR)/$(PACKAGE_NAME)/	
	-cd $(DIST_DIR); \
		tar -cvjf $(PACKAGE_NAME)-$(VERSION).tar.bz2  $(PACKAGE_NAME)/*     

distclean:
	-@rm -f $(DEPS)
	-@rm -f $(OUT)
	-@rm -f $(OBJS)
	-@rm $(TOP)/dist/$(PACKAGE_NAME)-$(VERSION).tar.bz2	
	-@rm -rf $(DIST_DIR)/$(PACKAGE_NAME)

clean:
	-@rm -f $(OUT)
	-@rm -f $(OBJS)

help:
	@echo "make (all):\t This is the default command when target unspecified "
	@echo "make clean:\t clean intermediate objects, target"
	@echo "make distclean:\t clean depends, intermediate objects, target"
