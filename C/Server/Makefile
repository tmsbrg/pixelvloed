SDIR	:=	./
IDIR 	:=	./
CC	:=	gcc

CFILES	:=	$(foreach dir,$(SDIR),$(wildcard $(dir)/*.c))
HFILES	:=	$(foreach dir,$(HDIR),$(wildcard $(dir)/*.h))

build:
	@echo
	@echo Compiling and linking files
	$(CC) $(CFILES) -I $(IDIR) -Os -o pixelvloed -Wall
	@echo Done
	@echo
	
	
