COMPILER := gcc
FLAGS	 := -lm -lpthread -O4 # -ggdb -Wall 
DEFINES  := -DNONE
# DEFINES := -DDEBUG
#DEFINES := -DBENCHMARK

all: matrixSum matrixSumV2 matrixSumV3 computePi

matrixSum: matrixSum.c
	$(COMPILER) $(DEFINES) -o $@ $^ $(FLAGS)

matrixSumV2: matrixSumV2.c
	$(COMPILER) $(DEFINES) -o $@ $^ $(FLAGS)

matrixSumV3: matrixSumV3.c
	$(COMPILER) $(DEFINES) -o $@ $^ $(FLAGS)

computePi: computePi.c
	$(COMPILER) $(DEFINES) -o $@ $^ $(FLAGS)
