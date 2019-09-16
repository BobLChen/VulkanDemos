#include "Engine.h"
#include "IOSAppDelegate.h"
#include "IOSViewController.h"
#include "Application/IOS/IOSWindow.h"

#include <vector>
#include <string>

@implementation IOSAppDelegate

+ (IOSAppDelegate*)GetDelegate
{
    return (IOSAppDelegate*)[UIApplication sharedApplication].delegate;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    VulkanViewController* viewController = [VulkanViewController alloc];
    g_IOSViewController = (__bridge void*)viewController;
    
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.screen = [UIScreen mainScreen];
    self.window.backgroundColor = [UIColor grayColor];
    self.window.rootViewController = viewController;
    
    [self.window makeKeyAndVisible];
    
    return YES;
}

- (void) didRotate:(NSNotification *)notification
{
    
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    Engine::Get()->RequestExit(true);
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    
}

- (void)application:(UIApplication *)application handleEventsForBackgroundURLSession:(NSString *)identifier completionHandler:(void (^)(void))completionHandler
{
    
}

@end
