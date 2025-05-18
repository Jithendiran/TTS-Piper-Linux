CC=g++
LIB_NAME=libttspiper
.PHONY: all clean module dymanic static
BUILD = build
dir_guard = mkdir -p $(BUILD)
EXCLUDE_DIRS = include $(BUILD)
EXCLUDE_PATTERNS = $(addsuffix /, $(EXCLUDE_DIRS)) .*/

SUBDIRS = $(filter-out $(EXCLUDE_PATTERNS), $(wildcard */))
CORE_OBJ = core/$(BUILD)/*.o
CTRL_OBJ = controller/$(BUILD)/*.o

all:dymanic

test-dymanic:dymanic
	$(CC) test.cpp  -L./$(BUILD) -lttspiper -Icore/include -Icontroller/include -o $(BUILD)/test.out
	@export LD_LIBRARY_PATH=$(BUILD):$LD_LIBRARY_PATH
	$(BUILD)/test.out

test-static:static
	$(CC) test.cpp  -L./$(BUILD) -lttspiper -Icore/include -Icontroller/include -o $(BUILD)/test.out
	$(BUILD)/test.out

dymanic:module
	$(dir_guard)
	$(CC) -shared  $(wildcard $(CORE_OBJ)) $(wildcard $(CTRL_OBJ)) -o $(BUILD)/$(LIB_NAME).so

static:module
	$(dir_guard)
	ar rcs $(BUILD)/$(LIB_NAME).a $(wildcard $(CORE_OBJ)) $(wildcard $(CTRL_OBJ))

module:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	rm -rf $(BUILD)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
