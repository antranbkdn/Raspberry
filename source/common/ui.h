#ifndef UI_H
#define UI_H

typedef void (*ui_print_callback_t) (void*);

extern ui_print_callback_t print_device_callback;

void set_print_callback(ui_print_callback_t your_callback);

#endif // UI_H
