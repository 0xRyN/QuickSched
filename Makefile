
CC = gcc
SDL_INC = -I/usr/include/SDL2
SDL_LIBS = -lSDL2 -lm

CFLAGS = -pthread -Wall -I. -g $(SDL_INC)

LDFLAGS = $(SDL_LIBS) -pthread

SRC_DIR = src
OBJ_DIR = obj

FRACTAL_DIR = $(SRC_DIR)/fractal
QUICKSORT_DIR = $(SRC_DIR)/quicksort

FRACTAL_SOURCES = $(shell ls $(FRACTAL_DIR)/*.c)
FRACTAL_OBJECTS = $(FRACTAL_SOURCES:%.c=$(OBJ_DIR)/%.o)

QUICKSORT_SOURCES = $(shell ls $(QUICKSORT_DIR)/*.c)
QUICKSORT_OBJECTS = $(QUICKSORT_SOURCES:%.c=$(OBJ_DIR)/%.o)

all:
	@echo "Usage: make <target>"
	@echo "Targets:"
	@echo "  fracsteal: Fractal with work-stealing scheduling"
	@echo "  quicklifo: Quicksort with LIFO scheduling"
	@echo "  quicksteal: Quicksort with work-stealing scheduling"
	@echo "  stealbench: Benchmark for quicksort with work-stealing scheduling"

fracsteal: OBJ_DIR workstealing_sched_sync.o $(FRACTAL_OBJECTS)
	$(CC) $(CFLAGS) $(FRACTAL_OBJECTS) $(OBJ_DIR)/workstealing_sched_sync.o -o fracsteal $(LDFLAGS)

quicklifo: OBJ_DIR lifo_sched.o $(QUICKSORT_OBJECTS)
	$(CC) $(CFLAGS) $(QUICKSORT_OBJECTS) $(OBJ_DIR)/lifo_sched.o -o quicklifo $(LDFLAGS)

quicksteal: OBJ_DIR workstealing_sched.o $(QUICKSORT_OBJECTS)
	$(CC) $(CFLAGS) $(QUICKSORT_OBJECTS) $(OBJ_DIR)/workstealing_sched.o -o quicksteal $(LDFLAGS)

stealbench: OBJ_DIR workstealing_sched_bench.o $(QUICKSORT_OBJECTS)
	$(CC) $(CFLAGS) $(QUICKSORT_OBJECTS) $(OBJ_DIR)/workstealing_sched_bench.o -o stealbench $(LDFLAGS)

lifo_sched.o: $(SRC_DIR)/lifo_sched.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/lifo_sched.c -o $(OBJ_DIR)/lifo_sched.o

workstealing_sched.o: $(SRC_DIR)/workstealing_sched.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/workstealing_sched.c -o $(OBJ_DIR)/workstealing_sched.o

workstealing_sched_sync.o: $(SRC_DIR)/workstealing_sched_sync.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/workstealing_sched_sync.c -o $(OBJ_DIR)/workstealing_sched_sync.o

workstealing_sched_bench.o: $(SRC_DIR)/workstealing_sched_bench.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/workstealing_sched_bench.c -o $(OBJ_DIR)/workstealing_sched_bench.o

OBJ_DIR: 
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -rf $(OBJ_DIR/*)
	rm -f fracsteal quicklifo quicksteal stealbench

.PHONY: all clean
