#import "VideoCompressPlugin.h"
#import <video_compress/video_compress-Swift.h>

@implementation VideoCompressPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftVideoCompressPlugin registerWithRegistrar:registrar];
}
@end
