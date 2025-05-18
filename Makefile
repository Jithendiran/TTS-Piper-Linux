.PHONY: all clean
EXCLUDE_DIRS = include build .git .vscode
SUBDIRS = $(filter-out $(EXCLUDE_DIRS), $(wildcard */))

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

	# generate static or shared lib

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
