#
# VM Main Makefile
#

.PHONY: all clean

all: 
	@make -C assembler
	@make -C vm

clean:
	@make -C assembler clean
	@make -C vm clean
