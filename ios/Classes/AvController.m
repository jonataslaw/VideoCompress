#import "AVController.h"
#import "Utility.h"
#import <AVFoundation/AVFoundation.h>
#import <MobileCoreServices/MobileCoreServices.h>

@implementation AVController

+ (AVURLAsset *)getVideoAsset:(NSURL *)url {
    return [AVURLAsset assetWithURL:url];
}

+ (AVAssetTrack *)getTrack:(AVURLAsset *)asset {
    __block AVAssetTrack* track = nil;
    dispatch_group_t group = dispatch_group_create();
    dispatch_group_enter(group);
    NSArray *array = [NSArray arrayWithObject:@"tracks"];
    [asset loadValuesAsynchronouslyForKeys:array completionHandler:^(void) {
        AVKeyValueStatus status = [asset statusOfValueForKey:@"tracks" error:nil];
        if (status == AVKeyValueStatusLoaded) {
            track = [asset tracksWithMediaType:AVMediaTypeVideo].firstObject;
        }
        dispatch_group_leave(group);
    }];
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    return track;
}

+ (NSInteger)getVideoOrientation:(NSString *)path {
    NSURL *url = [Utility getPathUrl:path];
    AVURLAsset *asset = [self getVideoAsset:url];
    AVAssetTrack *track = nil;
    if ([self getTrack:asset]) {
        track = [self getTrack:asset];
    }
    CGSize size = track.naturalSize;
    CGAffineTransform txf = track.preferredTransform;
    NSInteger orientation = 0;
    if (size.width == txf.tx && size.height == txf.ty) {
        orientation = 0;
    } else if (txf.tx == 0 && txf.ty == 0) {
        orientation = 90;
    } else if (txf.tx == 0 && txf.ty == size.width) {
        orientation = 180;
    } else {
        orientation = 270;
    }
    return orientation;
}

+ (NSString *)getMetaDataByTag:(AVAsset *)asset key:(NSString *)key {
    for (AVMetadataItem* item in asset.commonMetadata) {
        if (item.commonKey.accessibilityValue == key) {
            if (item.stringValue == (id)[NSNull null] || item.stringValue.length == 0 ) {
                return item.stringValue;
            } else {
                return @"";
            }
        }
    }
    return @"";
}
@end

