#import <Cocoa/Cocoa.h>

#include "../messagebox.h"

void message_box(const std::string &title, const std::string &message)
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:[NSString stringWithUTF8String:title.c_str()]];
    [alert setInformativeText:[NSString stringWithUTF8String:message.c_str()]];
    [alert setAlertStyle:NSAlertStyleWarning];
    
    [alert runModal];
}