COMPILER := gcc
FLAGS	 := -lm -lpthread -ggdb -Wall -O4
# DEFINES  := -DNONE
DEFINES := -DDEBUG
#DEFINES := -DBENCHMARK

all: uni-bath

uni-bath: uni-bath.c
	$(COMPILER) $(DEFINES) -o $@ $^ $(FLAGS)
