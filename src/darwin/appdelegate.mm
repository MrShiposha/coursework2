#import "appdelegate.h"

#include "../ui.h"

#include <fstream>


@interface AppDelegate () {
}
@end

@implementation AppDelegate

-(void) quit: (id)sender {
    Ui::stop();
}


-(void) setup_main_menu {
    NSMenu *main_menu = [[NSMenu alloc] init];
    NSMenuItem *app_menu_item = [main_menu addItemWithTitle:@"coursework"
                                           action: nil
                                           keyEquivalent:@""];

    NSMenu *app_menu = [[NSMenu alloc] init];
    NSMenuItem *quit_item = [app_menu addItemWithTitle:@"Quit"
                                      action: @selector(terminate:)
                                      keyEquivalent:@"q"];
    [quit_item setKeyEquivalentModifierMask:NSCommandKeyMask];

    [app_menu_item setSubmenu:app_menu];

    [[NSApplication sharedApplication] setMainMenu: main_menu];
}

@end