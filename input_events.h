#ifndef __INPUT_EVENTS__
#define __INPUT_EVENTS__

#include <stdint.h>

typedef enum {
    kInputEventType_Nothing = 0,
    kInputEventType_MouseDown,
    kInputEventType_MouseUp,
    kInputEventType_MouseMove,
    kInputEventType_KeyDown,
    kInputEventType_KeyUp,
    kInputEventType_Quit        
} InputEventType;

struct InputEvent_t {
    InputEventType eventType;
    // mouse event
    uint32_t x, y;
    uint32_t button;
    // keyboard events
    uint32_t scanCode;
    uint32_t keyCode;
    // modifier keys
    uint32_t modifiers;
};
typedef struct InputEvent_t InputEvent;



#endif
