CC = gcc

CFLAGS = -g -Wall 

CLIBS = -pthread

HEADER_DIR = ../headers

BUILD_DIR = ../../build

OBJ_DIR = $(BUILD_DIR)/obj

BIN_DIR = $(BUILD_DIR)/bin

INCLUDES = -I$(HEADER_DIR)

MKDIR = mkdir -p

SRCS = $(shell ls *.c)

OBJS = $(SRCS:.c=.o)

OBJS_BUILD = $(shell ls $(OBJ_DIR)/*.o)

APP = log

all: dirs $(APP)

$(APP): $(OBJS)
	$(CC) $(CFLAGS) $(CLIBS) -o $(BIN_DIR)/$(APP) $(OBJS_BUILD) 

dirs:
	$(MKDIR) $(OUT_DIR)	$(OBJ_DIR) $(BIN_DIR)

%.o: %.c $(HEADER_DIR)/*.h 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(OBJ_DIR)/$@

clean:
	rm -rf $(BUILD_DIR)
