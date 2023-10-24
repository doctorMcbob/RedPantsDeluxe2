# A simple Makefile for SDL projects
# set the compiler
CC := gcc

# set compiler flags 
SDL_FLAGS := $(shell pkg-config --static --cflags --libs /usr/local/lib/pkgconfig/sdl2.pc)
TTF_FLAGS := -L/usr/lib/x86_64-linux-gnu -lSDL2_ttf
CFLAGS := $(SDL_FLAGS) $(TTF_FLAGS) -ggdb3 -O0 --std=c99 -Wall -lm -Wl,-Bdynamic

# add headers here
HDRS :=

# add source files here
SRCS := $(wildcard $(C_DIR)/*.c)

# generate names of object files
OBJS := $(SRCS:.c=.o)

# name of executable
EXEC := game

# default recipe
all: $(EXEC)

# recipe for building the final executable
$(EXEC): $(OBJS) $(HDRS) 
	$(CC) -o $(BUILD_DIR)/$@ $(OBJS) $(CFLAGS) -I./include -L./lib -lSDL2 -lSDL2_image -lSDL2_mixer 

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
