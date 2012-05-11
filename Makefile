OS_NAME=$(shell uname -s)
LIBS+=-lm -ldl -lpthread
ifeq (${OS_NAME},Darwin)
LDFLAGS+=-framework CoreServices
else ifeq (${OS_NAME},Linux)
LDFLAGS+=-Wl,-E
LIBS+=-lrt
endif

# verbose build
export Q=
MAKEFLAGS+=-e

all: db

libuv/uv.a: libuv
	make -C libuv

db.o: db.c
	$(CC) -c db.c -o db.o -I libuv/include

db: db.o
	$(CC) db.o libuv/uv.a $(LIBS) -o db