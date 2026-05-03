#ifndef SW_INPUT_HANDLER_H
#define SW_INPUT_HANDLER_H

#include <gtk/gtk.h>
#include "include/ui/sw_switcher_widget.h"

gboolean sw_input_handler_handle_key_press(struct sw_switcher_widget *sw, GdkEventKey *event);

gboolean sw_input_handler_handle_release(struct sw_switcher_widget *sw, GdkEventKey *event);

#endif