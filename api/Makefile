CC = g++
INCLUDE_DIRS = $(wildcard ../*/include)

CPPFLAGS = -c $(addprefix -I, $(INCLUDE_DIRS)) -fPIC

BUILD = build
dir_guard = mkdir -p $(BUILD)
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, build/%.o, $(SRC))

.PHONY: clean all

all: $(OBJ)

$(BUILD)/%.o: %.cpp
	$(dir_guard)
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(BUILD)
