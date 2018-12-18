#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#import "appdelegate.h"

static AppDelegate *delegate = nullptr;

void set_correct_working_directory()
{
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *executable_path = [[bundle bundlePath] stringByAppendingString:@"/Contents/MacOS"]; 
    NSFileManager *file_manager = [NSFileManager defaultManager];
  
    [file_manager changeCurrentDirectoryPath:executable_path];
}

void setup_nsapp()
{
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
}

void setup_app_delegate() 
{
    delegate = [[AppDelegate alloc] init];
    [delegate setup_main_menu];
}

void initialize()
{
    set_correct_working_directory();
    setup_nsapp();
    setup_app_delegate();
}
