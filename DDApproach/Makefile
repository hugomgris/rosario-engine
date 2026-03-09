CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -I./srcs -I./srcs/components -I./srcs/systems -I./srcs/ecs -I./srcs/arena -I./srcs/data -I./srcs/AI -I./srcs/graphics -I./srcs/core
SRC = $(wildcard srcs/**/*.cpp)
OBJ = $(SRC:.cpp=.o)
EXEC = rosario-engine

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean