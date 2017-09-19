CC = g++
INC = 
LIB = -lpthread
SOURCE = $(wildcard *.cc)
OBJS = $(patsubst %.cc,%.o,$(SOURCE))
OUT = netcc
all:main
%.o:%.cc
	$(CC) $(INC) $(LIB) -g -c $< -o $*.o
main:$(OBJS)
	$(CC) $(INC) $(LIB) -g -o $(OUT)  $(OBJS)
clean:
	rm *.o -rf
	rm $(OUT) -rf
