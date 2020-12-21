#import <AVFoundation/AVFoundation.h>

@interface AvController : NSObject

+ (AVURLAsset *)getVideoAsset:(NSURL *)url;

+ (AVAssetTrack *)getTrack:(AVURLAsset *)asset;

+ (NSInteger)getVideoOrientation:(NSString *)path;

+ (NSString *)getMetaDataByTag:(AVAsset *)asset key:(NSString *)key;

@end


