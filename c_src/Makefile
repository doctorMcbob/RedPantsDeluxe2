# A simple Makefile for SDL projects

# set the compiler
CC := clang

# set compiler flags #! REMOVE -g !# 
SDL_FLAGS := `sdl2-config --libs --cflags`
TTF_FLAGS := -L/usr/lib/x86_64-linux-gnu -lSDL2_ttf
CFLAGS := $(SDL_FLAGS) $(TTF_FLAGS) -ggdb3 -O0 --std=c99 -Wall

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
	$(CC) -o $(BUILD_DIR)/$@ $(OBJS) $(CFLAGS) -lSDL2_image -lm

# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#	$(CC) -c $(CFLAGS) -o $(BUILD_DIR)/$@ $(@:.o=.c)

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
