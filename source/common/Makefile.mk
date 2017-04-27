CXXFLAGS	+= -I./source/common

VPATH += source/common

OBJ += $(OBJ_DIR)/cmd_line.o
OBJ += $(OBJ_DIR)/jsmn.o
OBJ += $(OBJ_DIR)/firmware.o
OBJ += $(OBJ_DIR)/slist.o
OBJ += $(OBJ_DIR)/random.o
OBJ += $(OBJ_DIR)/mac_address.o
OBJ += $(OBJ_DIR)/ui.o
OBJ += $(OBJ_DIR)/api.o

