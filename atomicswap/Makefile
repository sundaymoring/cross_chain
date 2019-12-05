SRCS = $(wildcard *.cpp)

OBJS = $(SRCS:.c = .o)

CC = g++ -std=c++11

INCLUDES = \
		-I"/usr/local/include/openssl"\
		-I"/home/hzy/work/c++/c++zzj/ato/jsonrpc-cpp"

OPENSSL_LIB_PATH = /usr/local/lib/openssl
LIBS = $(addprefix -L, $(OPENSSL_LIB_PATH))
LIBS = \
		-l crypto \
		-l ssl \
		-l boost_system \
		-l boost_thread \
		-l pthread \
		-l json \
		-l jsonrpc

CCFLAGS = -g -Wall -O0

OUTPUT = atomicswap

all:$(OUTPUT)

$(OUTPUT) : $(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS)

%.o : %.cpp
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -rf *.out *.o

.PHONY:clean

test:
	@echo $(OBJS)
