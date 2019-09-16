#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/IOS/IOSAppDelegate.h"
#include <UIKit/UIKit.h>
#include <stdio.h>

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([IOSAppDelegate class]));
    }
}
