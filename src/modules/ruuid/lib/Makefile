# Makefile to build the libs
#

include Makefile.defs

ifeq ($(OS),darwin)
LD_EXTRA_FLAGS= -framework Security
SOBASENAME=libruuid.dylib
else
ifeq ($(OS),linux)
LD_EXTRA_FLAGS= -lpthread
endif
SOBASENAME=libruuid.so
endif

ABASENAME=libruuid.a

.PHONY: all
all: lib

.PHONY: lib
lib:
	cargo build --release

.PHONY: example
example:
	gcc -o examples/ruuid -I include/ examples/ruuid.c target/release/libruuid.a $(LD_EXTRA_FLAGS)

.PHONY: install-dirs
install-dirs:
	mkdir -p ${DESTDIR}${PREFIX}
	mkdir -p ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${PREFIX}/lib
	mkdir -p ${DESTDIR}${PREFIX}/lib/pkgconfig
	mkdir -p ${DESTDIR}${PREFIX}/include

.PHONY: install-lib
install-lib:
	cp include/ruxc.h ${DESTDIR}${PREFIX}/include/
	cp target/release/${ABASENAME} ${DESTDIR}${PREFIX}/lib/
	cp target/release/${SOBASENAME} ${DESTDIR}${PREFIX}/lib/
	cp misc/pkg-config/libruxc.pc ${DESTDIR}${PREFIX}/lib/pkgconfig/

.PHONY: install
install: install-dirs install-lib

.PHONY: uninstall
uninstall:
	rm -f ${DESTDIR}${PREFIX}/include/ruuid.h
	rm -f ${DESTDIR}${PREFIX}/lib/${ABASENAME}
	rm -f ${DESTDIR}${PREFIX}/lib/${SOBASENAME}
	rm -f ${DESTDIR}${PREFIX}/lib/pkgconfig/libruuid.pc

.PHONY: clean
clean:
	rm -rf target
	rm -f examples/ruuid