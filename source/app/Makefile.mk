-include source/app/mqtt/Makefile.mk
CXXFLAGS	+= -I./source/app

VPATH += source/app

OBJ += $(OBJ_DIR)/app.o
OBJ += $(OBJ_DIR)/app_data.o

OBJ += $(OBJ_DIR)/if_console.o
OBJ += $(OBJ_DIR)/if_rf433.o
OBJ += $(OBJ_DIR)/if_rf24.o

OBJ += $(OBJ_DIR)/task_list.o
OBJ += $(OBJ_DIR)/task_console.o
OBJ += $(OBJ_DIR)/task_snmp.o
OBJ += $(OBJ_DIR)/task_rf433.o
OBJ += $(OBJ_DIR)/task_mqtt.o
OBJ += $(OBJ_DIR)/task_firmware.o
OBJ += $(OBJ_DIR)/task_if.o
OBJ += $(OBJ_DIR)/task_sensor.o
OBJ += $(OBJ_DIR)/task_sm.o
OBJ += $(OBJ_DIR)/task_digital_device.o
OBJ += $(OBJ_DIR)/task_query_address.o
OBJ += $(OBJ_DIR)/task_register_device.o
OBJ += $(OBJ_DIR)/task_init_gateway.o

