#include "ui.h"

ui_print_callback_t print_device_callback;

void set_print_callback(ui_print_callback_t your_callback) {
    print_device_callback = your_callback;
}
