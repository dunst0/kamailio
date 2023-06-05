#
# ruuid module makefile
#
# WARNING: do not run this directly, it should be run by the main Makefile

include ../../Makefile.defs
auto_gen=
NAME=ruuid.so

RUUID_PREFIX = ./lib/target
RUUID_SRC = ./lib

LIBS += $(RUUID_PREFIX)/release/libruuid.a -lpthread
INCLUDES += -I$(RUUID_SRC)/include/

ifeq ($(OS), darwin)
LIBS+= -framework CoreFoundation -framework Security
endif

include ../../Makefile.modules

$(RUUID_PREFIX)/release/libruuid.a:
	@$(MAKE) -C $(RUUID_SRC)

$(NAME): $(RUUID_PREFIX)/release/libruuid.a

clean-ruuid:
	@rm -rf $(RUUID_PREFIX)

clean-module:
	@rm -f *.o *.so

clean-all: clean-ruuid clean-module

.PHONY: clean-ruuid clean-module clean-all
