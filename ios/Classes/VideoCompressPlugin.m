#import "VideoCompressPlugin.h"
#import "Utility.h"
#import <AVFoundation/AVFoundation.h>

@interface VideoCompressPlugin ()

- (void)compressVideo:(NSString *)path quality:(NSNumber *)quality result:(FlutterResult)result;
- (void)cancelCompression:(FlutterResult)result;
- (NSDictionary *)getMediaInfoJson:(NSString *)path;
- (NSString *)getExportPreset:(NSNumber *)quality;

@end

@implementation VideoCompressPlugin {
    AVAssetExportSession *exporter;
    bool stopCommand;
}

+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
    FlutterMethodChannel *channel =
    [FlutterMethodChannel methodChannelWithName: @"video_compress"
                                binaryMessenger:[registrar messenger]];
    VideoCompressPlugin *instance = [VideoCompressPlugin new];
    [registrar addMethodCallDelegate:instance channel:channel];
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    if ([@"compressVideo" isEqualToString:call.method]) {
        NSString *path = args[@"path"];
        NSNumber *quality = args[@"quality"];
        [self compressVideo:path quality:quality result:result];
    } else if ([@"cancelCompression" isEqualToString:call.method]) {
        [self cancelCompression:result];
    } else if ([@"deleteAllCache" isEqualToString:call.method]) {
        [Utility deleteFile:[Utility basePath]];
        result(@YES);
    } else {
        result(FlutterMethodNotImplemented);
    }
}

- (NSDictionary *)getMediaInfoJson:(NSString *)path {
    NSDictionary* dictionary = @{ @"path": [Utility excludeFileProtocol:path] };
    return dictionary;
}

- (NSString *)getExportPreset:(NSNumber *)quality {
    switch ([quality intValue]) {
        case 1:
            return AVAssetExportPresetLowQuality;
        case 2:
            return AVAssetExportPresetMediumQuality;
        case 3:
            return AVAssetExportPresetHighestQuality;
        default:
            return AVAssetExportPresetMediumQuality;
    }
}

- (void)compressVideo:(NSString *)path quality:(NSNumber *)quality result:(FlutterResult)result {
    NSURL *sourceVideoUrl = [Utility getPathUrl:path];
    AVURLAsset *sourceVideoAsset = [AVURLAsset URLAssetWithURL:sourceVideoUrl options:nil];
    
    NSURL *compressionUrl = [Utility getPathUrl:[NSString stringWithFormat:@"%@/%@.mp4", [Utility basePath], [Utility getFileName:path]]];
    
    stopCommand = false;
    exporter = [AVAssetExportSession exportSessionWithAsset:sourceVideoAsset presetName: [self getExportPreset:quality]];
    exporter.outputURL = compressionUrl;
    exporter.outputFileType = AVFileTypeMPEG4;
    exporter.shouldOptimizeForNetworkUse = true;
    
    [Utility deleteFile:[compressionUrl absoluteString]];
    
    [exporter exportAsynchronouslyWithCompletionHandler: ^{
        self->exporter = nil;
        if (self->stopCommand){
            self->stopCommand = false;
            NSMutableDictionary *json = [[self getMediaInfoJson:path] mutableCopy];
            [json setValue:@YES forKey:@"isCancel"];
            NSString *jsonString = [Utility keyValueToJson:json];
            return result(jsonString);
        }
        NSMutableDictionary *json = [[self getMediaInfoJson:[compressionUrl absoluteString]] mutableCopy];
        [json setValue:@NO forKey:@"isCancel"];
        NSString *jsonString = [Utility keyValueToJson:json];
        return result(jsonString);
    }];
}

- (void)cancelCompression:(FlutterResult)result {
    if (exporter != nil) {
        [exporter cancelExport];
        stopCommand = true;
    }
    result(@"");
}

@end

