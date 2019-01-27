CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++17 -ggdb -lcommoncpp -lucommon -lrt -ldl -lgcrypt -lgpg-error
LDFLAGS	  := -lcommoncpp -lucommon -lrt -ldl -lgcrypt -lgpg-error

BIN		:= bin
SRC		:= src
INCLUDE	:= include
LIB		:= lib

LIBRARIES	:= lib/libccrtp.so lib/libcommoncpp.so lib/libucommon.so
EXECUTABLE	:= main


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*
