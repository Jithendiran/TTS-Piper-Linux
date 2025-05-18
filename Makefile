CC=g++
AR=ar
LIB_NAME=libttspiper
.PHONY: all clean module dymanic static test-dymanic test-static
BUILD = build
dir_guard = mkdir -p $(BUILD)
EXCLUDE_DIRS = include $(BUILD)
EXCLUDE_PATTERNS = $(addsuffix /, $(EXCLUDE_DIRS)) .*/

SUBDIRS = $(filter-out $(EXCLUDE_PATTERNS), $(wildcard */))
CORE_OBJ = core/$(BUILD)/*.o
CTRL_OBJ = controller/$(BUILD)/*.o

all:$(BUILD)/$(LIB_NAME).so

test-dymanic:$(BUILD)/$(LIB_NAME).so $(BUILD)/test.out
	@export LD_LIBRARY_PATH=$(BUILD):$LD_LIBRARY_PATH
	$(BUILD)/test.out

test-static:$(BUILD)/$(LIB_NAME).a $(BUILD)/test.out
	$(BUILD)/test.out

$(BUILD)/test.out: test.cpp $(BUILD)/libttspiper.so
	$(CC) test.cpp -L./$(BUILD) -lttspiper -Icore/include -o $(BUILD)/test.out

$(BUILD)/$(LIB_NAME).so:module
	$(dir_guard)
	$(CC) -shared  $(wildcard $(CORE_OBJ)) $(wildcard $(CTRL_OBJ)) -o $(BUILD)/$(LIB_NAME).so

$(BUILD)/$(LIB_NAME).a:module
	$(dir_guard)
	$(ar) rcs $(BUILD)/$(LIB_NAME).a $(wildcard $(CORE_OBJ)) $(wildcard $(CTRL_OBJ))

module:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	rm -rf $(BUILD)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
