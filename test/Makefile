CC = gcc
CPP = g++
BUILD = build
MOD_BUILD = ../build
dir_guard = mkdir -p $(BUILD)
INCLUDE = -I../api/include -I../core/include -I../controller/include 
LIBS = -lttspiper -L$(MOD_BUILD)
FLAG=

C_TEST_SRC = test_api.c
CPP_TEST_SRC = test.cpp

C_TEST_OUT = test_api.out
CPP_TEST_OUT = test.out

.PHONY: all run-c run-cpp clean

all: $(C_TEST_OUT) $(CPP_TEST_OUT)

debug:
	$(MAKE) DEBUG=1 FLAG="-g" all

$(C_TEST_OUT): $(C_TEST_SRC)
	$(dir_guard)
	$(CC) $(FLAG) $(C_TEST_SRC) $(INCLUDE) $(LIBS) -o $(BUILD)/$(C_TEST_OUT)

$(CPP_TEST_OUT): $(CPP_TEST_SRC)
	$(dir_guard)
	$(CPP) $(FLAG) $(CPP_TEST_SRC) $(INCLUDE) $(LIBS) -o $(BUILD)/$(CPP_TEST_OUT)

run-c: $(C_TEST_OUT)
	LD_LIBRARY_PATH=$(MOD_BUILD):$$LD_LIBRARY_PATH $(BUILD)/$(C_TEST_OUT)

run-cpp: $(CPP_TEST_OUT)
	LD_LIBRARY_PATH=$(MOD_BUILD):$$LD_LIBRARY_PATH $(BUILD)/$(CPP_TEST_OUT)

clean:
	rm -rf $(BUILD)
