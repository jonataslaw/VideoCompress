#import "AvController.h"
#import <AVFoundation/AVFoundation.h>
#import <MobileCoreServices/MobileCoreServices.h>

@implementation AvController

+ (AVURLAsset *)getVideoAsset:(NSURL *)url {
    return [AVURLAsset assetWithURL:url];
}

+ (AVAssetTrack *)getTrack:(AVURLAsset *)asset {
    __block AVAssetTrack* track = nil;
    dispatch_group_t group = dispatch_group_create();
    dispatch_group_enter(group);
    NSArray *array = [NSArray arrayWithObject:@"tracks"];
    [asset loadValuesAsynchronouslyForKeys:array completionHandler:^(void){
        AVKeyValueStatus status = [asset statusOfValueForKey:@"tracks" error:nil];
        if(status == AVKeyValueStatusLoaded) {
            track = [asset tracksWithMediaType:AVMediaTypeVideo].firstObject;
        }
        dispatch_group_leave(group);
    }];
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    return track;
}


+ (NSInteger *)getVideoOrientation:(NSString *)path {
    NSInteger *temp = 0;
    return temp;
}

+ (NSString *)getMetaDataByTag:(AVAsset *)asset key:(NSString *)key {
    for (AVMetadataItem* item in asset.commonMetadata) {
        if(item.commonKey.accessibilityValue == key) {
            if (item.stringValue == (id)[NSNull null] || item.stringValue.length == 0 )
                return item.stringValue;
            else
                return @"";
        }
    }
    return @"";
}
@end
