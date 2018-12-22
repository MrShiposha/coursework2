#import "windowdelegate.h"

@interface WindowDelegate() {
    std::function<void()>                 resize_callback;
    std::function<void(int32_t, int32_t)> mouse_moved_callback;
    std::function<void(MouseButton)>      mouse_down_callback;
    std::function<void(MouseButton)>      mouse_up_callback;
    std::function<void(const Key &)>     key_callback;

    NSTrackingArea *tracking_area;
}
@end

@implementation WindowDelegate

-(void) init_default_callbacks {
    resize_callback      = [] {};
    mouse_moved_callback = [](int32_t, int32_t) {};
    mouse_down_callback  = [](MouseButton) {};
    mouse_up_callback    = [](MouseButton) {};
    key_callback         = [](const Key &) {};
}

-(void) set_resize_callback: (std::function<void()>)callback {
    resize_callback = callback;
}

-(void) set_mouse_move_callback: (std::function<void(int32_t, int32_t)>)callback {
    mouse_moved_callback = callback;
}

-(void) set_mouse_down_callback: (std::function<void(MouseButton)>)callback {
    mouse_down_callback = callback;
}

-(void) set_mouse_up_callback: (std::function<void(MouseButton)>)callback {
    mouse_up_callback = callback;
}

-(void) set_key_callback: (std::function<void(const Key &)>)callback {
    key_callback = callback;
}

- (BOOL)acceptsFirstResponder {
   return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent {
    return YES;
}

-(void)keyDown:(NSEvent *)event {
    if(event.type == NSEventTypeKeyDown) {
        const char *chars = [event.charactersIgnoringModifiers UTF8String];
        if(chars[0] != '\0')
        {
            Key key = {};
            key.code = chars[0];
            key.state = Key::State::PRESSED;

            if(event.modifierFlags & NSEventModifierFlagCommand) {
                key.modifiers = Key::Modifiers::COMMAND;
            }

            key_callback(key);
        }
    }
}

-(void)keyUp:(NSEvent *)event {
    if(event.type == NSEventTypeKeyUp) {
        const char *chars = [event.charactersIgnoringModifiers UTF8String];
        if(chars[0] != '\0')
        {
            Key key = {};
            key.code = chars[0];
            key.state = Key::State::RELEASED;

            if(event.modifierFlags & NSEventModifierFlagCommand) {
                key.modifiers = Key::Modifiers::COMMAND;
            }

            key_callback(key);
        }
    }
}

-(void)mouseDown:(NSEvent *)event {    
    mouse_down_callback(MouseButton::LEFT);
}

-(void)mouseUp:(NSEvent *)event {    
    mouse_up_callback(MouseButton::LEFT);
}

- (void)rightMouseDown: (NSEvent*)event {
    mouse_down_callback(MouseButton::RIGHT);
}

- (void)rightMouseUp: (NSEvent*)event {
    mouse_up_callback(MouseButton::RIGHT);
}

-(void)mouseMoved:(NSEvent *)event {   
    NSPoint position = [self convertPointFromScreen:[NSEvent mouseLocation]];
    CGSize view_size = [self contentView].frame.size;

    if
    (
        position.x < 0 
        || position.x > view_size.width 
        || position.y < 0 
        || position.y > view_size.height
    ) return;

    mouse_moved_callback(static_cast<int32_t>(position.x), static_cast<int32_t>(position.y));
}

-(NSSize)windowWillResize:(NSWindow *)sender
                   toSize:(NSSize)frameSize {
    [self setContentSize: [self contentRectForFrameRect:self.frame].size];

    resize_callback();

    return frameSize;
}

@end
