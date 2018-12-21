#include <Cocoa/Cocoa.h>

#include <functional>

#include "../mousebutton.h"
#include "../key.h"

@interface WindowDelegate : NSWindow<NSWindowDelegate>
-(void) init_default_callbacks;
-(void) set_resize_callback: (std::function<void()>)callback;
-(void) set_mouse_move_callback: (std::function<void(int32_t, int32_t)>)callback;
-(void) set_mouse_down_callback: (std::function<void(MouseButton)>)callback;
-(void) set_mouse_up_callback: (std::function<void(MouseButton)>)callback;
-(void) set_key_callback: (std::function<void(const Key &)>)callback;
@end