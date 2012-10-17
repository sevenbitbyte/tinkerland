CXX=g++
CXXFLAGS = -ggdb
CFLAGS = -ggdb
LDFLAGS=-lm
CXXLIBS= -lm

LIB_FILES = icpmatcher.cpp kdnode.cpp point.cpp
LIB_OBJS = ${LIB_FILES:%.cpp=%.o}

TARGETS = libicp.a

all: ${TARGETS}

libicp.a: ${LIB_OBJS}
	rm -f $@
	ar ruP $@ ${LIB_OBJS}

clean:
	rm -f *.o
	rm -f ${TARGETS}
