#import <UIKit/UIKit.h>
#include <stdio.h>

int main(int argc, char * argv[]) {
    printf("argv:%s\n", argv[0]);
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, @"AppDelegate");
    }
}

