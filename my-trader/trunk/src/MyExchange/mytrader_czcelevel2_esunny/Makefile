# Makefile 

CC:=g++
DEBUG:=n
TOP:=../../../
VERSION:=v2.6.x
DIST_DIR:=$(TOP)/dist
# to be changed
PROJECT_NAME:=mytrader_czcelevel2_esunny

INCPATH:= -I$(TOP)/lib/ \
	  -I$(TOP)/lib/quote/include \
	  -I$(TOP)/lib/tunnel/include \
	  -I$(TOP)/lib/my_common \
	  -I../utils \
	  -I../tinyxml \
	  -I../te \
	  -I../my_exchange_db \
	  -I../tca \
	  -I../sm \
	  -I../my_exchange_entity \
	  -I../QA \
	  -I../$(PROJECT_NAME)

LIBPATH:=-L$(TOP)/lib/common \
		-L$(TOP)/lib/quote/czce_jr_udp_level2/bin \
		-L$(TOP)/lib/quote\
        -L../utils/bin \
	  	-L../tinyxml/bin \
	  	-L../te/bin \
	  	-L../tca/bin \
	  	-L../sm/bin \
	  	-L../QA/bin \
	  	-L../my_exchange_entity/bin \
	  	-L../my_exchange_db/bin	  	

CPPFLAGS:= -v -std=c++11 $(INCPATH)
CFLAGS:= 
LDFLAGS:= 
OBJPATH:=./obj
BINPATH:=./bin
LIBS:=  -lutils -ldl -lpthread -lboost_filesystem -lboost_system \
	-lboost_date_time -llog4cxx -lssl -ltca -lsm -lmy_common_v2 -lmy_exchange_entity \
	-ltinyxml -lmy_quote_tap_lib -lTapQuoteAPI  -lmy_quote_czce_level2_jrudp_lib -lqtm

DEPS:=.depends

# bo be changed
OUT:=czcelevel2_tradsvc
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
	-rm -fr $(DIST_DIR)/$(PACKAGE_NAME)
	-mkdir -p $(DIST_DIR)/$(PACKAGE_NAME)
	-mkdir -p $(DIST_DIR)/$(PACKAGE_NAME)/ev
	-mkdir -p $(DIST_DIR)/$(PACKAGE_NAME)/iv
	-mkdir -p $(DIST_DIR)/$(PACKAGE_NAME)/log
	-cp -varf $(CURDIR)/bin/* $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -varf $(CURDIR)/*.config $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -varf $(CURDIR)/*.key $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -varf $(CURDIR)/*.xml $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -varf $(TOP)/lib/intel64 $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -varf $(TOP)/lib/common/* $(DIST_DIR)/$(PACKAGE_NAME)/	
	-cp -vaf $(TOP)/lib/quote/czce_jr_udp_level2/bin/* $(DIST_DIR)/$(PACKAGE_NAME)/
	-cp -vaf $(TOP)/lib/quote/czce_jr_udp_level2/config/* $(DIST_DIR)/$(PACKAGE_NAME)/	
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
