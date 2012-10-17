BUILD_TYPE = -D BUILD_DEBUG

CC=gcc ${BUILD_TYPE}
CXX=g++ ${BUILD_TYPE}
LDFLAGS=-lm
CFLAGS=-Wall -ggdb
CXXFLAGS=${CFLAGS}


LIB_FILES = neuron.cpp dataset.cpp testdatasource.cpp electionDataSource.cpp utils.cpp
LIB_OBJS = ${LIB_FILES:%.cpp=%.o}

LOGIC_TEST_OBJS = logic_test.cpp ${LIB_OBJS}
NNTOOL_OBJS=nntool.cpp ${LIB_OBJS}
PREDICT_OBJS=predict.cpp ${LIB_OBJS}

TARGETS=nntool predict logic_test

all: ${TARGETS}

logic_test: ${LOGIC_TEST_OBJS}
	${CXX} ${LOGIC_TEST_OBJS} -o logic_test ${LDFLAGS}

nntool: ${NNTOOL_OBJS}
	${CXX} ${NNTOOL_OBJS}  -o nntool ${LDFLAGS}

predict: ${PREDICT_OBJS}
	${CXX} ${PREDICT_OBJS}  -o predict ${LDFLAGS}


clean:
	rm -f *.o *.a
	rm -f ${TARGETS}

