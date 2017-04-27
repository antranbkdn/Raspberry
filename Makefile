###############################################################################
# Author: ThanNT
# Date: 06/01/2017
###############################################################################

NAME_MODULE	= rf_24_service
OPTIMIZE	= -g -Os
CXX		= g++
CC		= gcc
OBJ_DIR		= $(NAME_MODULE)_build

-include source/ak/Makefile.mk
-include source/sys/Makefile.mk
-include source/app/Makefile.mk
-include source/common/Makefile.mk
-include source/driver/Makefile.mk

# CXX compiler option
CXXFLAGS	+=	$(OPTIMIZE)	\
		-std=c++11		\
		-Wall			\
		-Winline		\
		-pipe			\
		-g			\
		-DRPI

# Library paths
LDFLAGS	+= -L/usr/local/lib
LDFLAGS	+= -L/usr/include

#Library libs
LDLIBS	+=	-lpthread		\
		-lrt			\
		-lwiringPi		\
		-lwiringPiDev		\
		-lm			\
		-lrf24-bcm		\
		-lrf24network		\
		-lmosquittopp		\
		-lmosquitto

LDFLAGS += -L/usr/local/lib
LDFLAGS += -L/usr/include

all: create $(OBJ_DIR)/$(NAME_MODULE)

create:
	@echo mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: %.c
	@echo Compiling $<
	@$(CC) -c -o $@ $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/$(NAME_MODULE): $(OBJ)
	@echo ---------- START LINK PROJECT ----------
	@echo $(CXX) -o $@ $^ $(CXXFLAGS)
	@$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)
	@echo ---------- START RUN PROJECT ----------

copy:
	scp -r ./* pi@192.168.168.147:/home/pi/workspace/getway/

flash:
	@sudo $(OBJ_DIR)/$(NAME_MODULE)

debug:
	sudo gdb $(OBJ_DIR)/$(NAME_MODULE)

install:
	cp $(OBJ_DIR)/$(NAME_MODULE) /usr/local/bin

clean:
	@echo rm -rf $(OBJ_DIR)
	@rm -rf $(OBJ_DIR)
