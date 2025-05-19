CC=g++
AR=ar
LIB_NAME=libttspiper
.PHONY: all clean module dymanic static
BUILD = build
dir_guard = mkdir -p $(BUILD)
EXCLUDE_DIRS = include $(BUILD) doc test
EXCLUDE_PATTERNS = $(addsuffix /, $(EXCLUDE_DIRS)) .*/
FLAG=
SUBDIRS = $(filter-out $(EXCLUDE_PATTERNS), $(wildcard */))
CORE_OBJ = core/$(BUILD)/*.o
CTRL_OBJ = controller/$(BUILD)/*.o
API_OBJ = api/$(BUILD)/*.o

all:$(BUILD)/$(LIB_NAME).so

debug:
	$(MAKE) DEBUG=1 FLAG="-g" all

$(BUILD)/$(LIB_NAME).so:module
	$(dir_guard)
	$(CC) -shared $(FLAG) $(wildcard $(CORE_OBJ)) $(wildcard $(CTRL_OBJ)) $(wildcard $(API_OBJ)) -o $(BUILD)/$(LIB_NAME).so

$(BUILD)/$(LIB_NAME).a:module
	$(dir_guard)
	$(AR) rcs $(FLAG) $(BUILD)/$(LIB_NAME).a $(wildcard $(CORE_OBJ)) $(wildcard $(CTRL_OBJ))

module:
	for dir in $(SUBDIRS); do \
        if [ "$(DEBUG)" = "1" ]; then \
            $(MAKE) -C $$dir debug; \
        else \
            $(MAKE) -C $$dir; \
        fi \
    done

clean:
	rm -rf $(BUILD)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
