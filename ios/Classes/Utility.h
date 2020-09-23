NS_ASSUME_NONNULL_BEGIN

@interface Utility : NSObject

+ (NSString *)basePath;

+ (NSString *)excludeFileProtocol:(NSString *)path;

+ (NSURL *)getPathUrl:(NSString *)path;

+ (NSString *)stripFileExtension:(NSString *)fileName;

+ (NSString *)getFileName:(NSString *)path;

+ (void)deleteFile:(NSString *)path;

+ (NSString *)keyValueToJson:(NSDictionary *)keyAndValue;


@end

NS_ASSUME_NONNULL_END

