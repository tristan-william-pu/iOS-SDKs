/*
    File:  AVDepthData.h
 
    Framework:  AVFoundation
 
    Copyright 2016-2017 Apple Inc. All rights reserved.
*/

#import <AVFoundation/AVBase.h>
#import <AVFoundation/AVCameraCalibrationData.h>
#import <Foundation/Foundation.h>
#import <CoreVideo/CVPixelBufferPool.h>
#import <ImageIO/CGImageProperties.h>

NS_ASSUME_NONNULL_BEGIN

/*
 @enum AVDepthDataQuality
 @abstract
    Constants indicating the overall quality of depth data map values.
 
 @constant AVDepthDataQualityLow
    The overall quality of values within the map is low (too many invalid depth values).
 @constant AVDepthDataQualityHigh
    The overall quality of values within the map is high.
 
 @discussion
    Depth data maps are typically generated by comparing images and calculating disparity. If features are lacking in either input image, it may be difficult to find matching keypoints, resulting in a depth data map with substantial holes. These holes can be filled with depth data filtering, but still may produce a map of overall poor quality. If a depth data map suffers from insufficient features, it is marked as low quality, and should be considered a substandard candidate for rendering high-quality depth effects or reconstructing a 3D scene. High quality depth is feature-rich, contains a high level of detail, and should be considered a good candidate for rendering high-quality depth effects or reconstructing a 3D scene.
*/
typedef NS_ENUM(NSInteger, AVDepthDataQuality) {
    AVDepthDataQualityLow    = 0,
    AVDepthDataQualityHigh   = 1,
} API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0)) __WATCHOS_PROHIBITED;

/*
 @enum AVDepthDataAccuracy
 @abstract
    Constants indicating the accuracy of the units expressed by depth data map values.

 @constant AVDepthDataAccuracyRelative
    Values within the depth data map are usable for foreground / background separation, but are not absolutely accurate in the physical world.
 @constant AVDepthDataAccuracyAbsolute
    Values within the depth map are absolutely accurate within the physical world.
 
 @discussion
    The accuracy of a depth data map is highly dependent on the camera calibration data used to generate it. If the camera's focal length cannot be precisely determined at the time of capture, scaling error in the z (depth) plane will be introduced. If the camera's optical center can't be precisely determined at capture time, principal point error will be introduced, leading to an offset error in the disparity estimate. AVDepthDataAccuracy constants report the accuracy of a map's values with respect to its reported units. If the accuracy is reported to be AVDepthDataAccuracyRelative, the values within the map are usable relative to one another (that is, larger depth values are farther away than smaller depth values), but do not accurately convey real world distance. Disparity maps with relative accuracy may still be used to reliably determine the difference in disparity between two points in the same map.
*/
typedef NS_ENUM(NSInteger, AVDepthDataAccuracy) {
    AVDepthDataAccuracyRelative    = 0,
    AVDepthDataAccuracyAbsolute    = 1,
} API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0)) __WATCHOS_PROHIBITED;

@class AVDepthDataInternal;

/*!
 @class AVDepthData
 @abstract
    An object wrapping a map of disparity or depth pixel data, plus metadata.
 
 @discussion
    "Depth Data" is a generic term for a map of pixel data containing depth-related information. AVDepthData wraps a disparity or depth map and provides conversion methods, focus information, and camera calibration data to aid in using the map for rendering or computer vision tasks. CoreVideo supports the following four depth data pixel formats:
          kCVPixelFormatType_DisparityFloat16    = 'hdis'
          kCVPixelFormatType_DisparityFloat32    = 'fdis'
          kCVPixelFormatType_DepthFloat16        = 'hdep'
          kCVPixelFormatType_DepthFloat32        = 'fdep'
 
    The disparity formats describe normalized shift values when comparing two images. Units are 1/meters: ( pixelShift / (pixelFocalLength * baselineInMeters) ). 
    The depth formats describe the distance to an object in meters.
 
    Disparity / depth maps are generated from camera images containing non-rectilinear data. Camera lenses have small imperfections that cause small distortions in their resultant images compared to a pinhole camera. AVDepthData maps contain non-rectilinear (non-distortion-corrected) data as well. Their values are warped to match the lens distortion characteristics present in their accompanying YUV image. Therefore an AVDepthData map can be used as a proxy for depth when rendering effects to its accompanying image, but not to correlate points in 3D space. In order to use AVDepthData for computer vision tasks, you should use its accompanying camera calibration data to rectify the depth data (see AVCameraCalibrationData).
 
    When capturing depth data from a camera using AVCaptureDepthDataOutput, AVDepthData objects are delivered to your AVCaptureDepthDataOutputDelegate in a streaming fashion. When capturing depth data along with photos using AVCapturePhotoOutput, depth data is delivered to your AVCapturePhotoCaptureDelegate as a property of an AVCapturePhoto (see -[AVCapturePhotoCaptureDelegate captureOutput:didFinishProcessingPhoto:error:]). When working with image files containing depth information, AVDepthData may be instantiated using information obtained from ImageIO. When editing images containing depth information, derivative AVDepthData objects may be instantiated reflecting the edits that have been performed.
 */
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0)) __WATCHOS_PROHIBITED
@interface AVDepthData : NSObject
{
@private
    AVDepthDataInternal *_internal;
}

AV_INIT_UNAVAILABLE

/*!
 @method depthDataFromDictionaryRepresentation:error:
 @abstract
    Returns an AVDepthData instance from depth information in an image file.
 
 @param imageSourceAuxDataInfoDictionary
    A dictionary of primitive depth-related information obtained from CGImageSourceCopyAuxiliaryDataInfoAtIndex.
 @param outError
    On return, if the depth data cannot be created, points to an NSError describing the problem.
 @result
    An AVDepthData instance, or nil if the auxiliary data info dictionary was malformed.
 
 @discussion
    When using ImageIO framework's CGImageSource API to read from a HEIF or JPEG file containing depth data, AVDepthData can be instantiated using the result of CGImageSourceCopyAuxiliaryDataInfoAtIndex, which returns a CFDictionary of primitive map information.
 */
+ (nullable instancetype)depthDataFromDictionaryRepresentation:(NSDictionary *)imageSourceAuxDataInfoDictionary error:(NSError * _Nullable * _Nullable)outError;

/*!
 @method depthDataByConvertingToDepthDataType:
 @abstract
    Returns a converted, derivative AVDepthData instance in the specified depthDataType.
 
 @param depthDataType
    The OSType of depthData object to which you'd like to convert. Must be present in availableDepthDataTypes.
 @result
    An AVDepthData instance.
 
 @discussion
    This method throws an NSInvalidArgumentException if you pass an unrecognized depthDataType. See
 */
- (instancetype)depthDataByConvertingToDepthDataType:(OSType)depthDataType;

/*!
 @method depthDataByApplyingExifOrientation:
 @abstract
    Returns a derivative AVDepthData instance in which the specified Exif orientation has been applied.
 
 @param exifOrientation
    One of the 8 standard Exif orientation tags expressing how the depth data should be rotated / mirrored.
 @result
    An AVDepthData instance.
 
 @discussion
    When applying simple 90 degree rotation or mirroring edits to media containing depth data, you may use this initializer to create a derivative copy of the depth in which the specified orientation is applied to both the underlying pixel map data and the camera calibration data. This method throws an NSInvalidArgumentException if you pass an unrecognized exifOrientation.
 */
- (instancetype)depthDataByApplyingExifOrientation:(CGImagePropertyOrientation)exifOrientation;

/*!
 @method depthDataByReplacingDepthDataMapWithPixelBuffer:error:
 @abstract
    Returns an AVDepthData instance wrapping the replacement depth data map pixel buffer.
 
 @param pixelBuffer
    A pixel buffer containing depth data information in one of the 4 supported disparity / depth pixel formats.
 @param outError
    On return, if the depth data cannot be created, points to an NSError describing the problem.
 @result
    An AVDepthData instance, or nil if the pixel buffer is malformed.
 
 @discussion
    When applying complex edits to media containing depth data, you may create a derivative map with arbitrary transforms applied to it, then use this initializer to create a new AVDepthData. Note that this new depth data object has no camera calibration data, so its cameraCalibrationData property always returns nil.
 */
- (nullable instancetype)depthDataByReplacingDepthDataMapWithPixelBuffer:(CVPixelBufferRef)pixelBuffer error:(NSError * _Nullable * _Nullable)outError;

/*!
 @property availableDepthDataTypes
 @abstract
    Specifies which depth data pixel formats may be used with depthDataByConvertingToDepthDataType:.
 
 @discussion
    This property presents the available pixel format types as an array of NSNumbers, each wrapping an OSType (CV pixel format type).
 */
@property(readonly) NSArray<NSNumber *> *availableDepthDataTypes;

/*!
 @method dictionaryRepresentationForAuxiliaryDataType:
 @abstract
    Returns a dictionary of primitive map information to be used when writing an image file with depth data.
 
 @param outAuxDataType
    On output, either kCGImageAuxiliaryDataTypeDisparity or kCGImageAuxiliaryDataTypeDepth, depending on the depth data's file.
 @result
    A dictionary of CGImageDestination compatible depth information, or nil if the auxDataType is unsupported.
 
 @discussion
    When using ImageIO framework's CGImageDestination API to write depth data to a HEIF or JPEG file, you may use this method to generate a dictionary of primitive map information consumed by CGImageDestinationAddAuxiliaryDataInfo.
 */
- (nullable NSDictionary *)dictionaryRepresentationForAuxiliaryDataType:(NSString * _Nullable * _Nullable)outAuxDataType;

/*!
 @property depthDataType
 @abstract
    Specifies the pixel format type of this depth data object's internal map.
 
 @discussion
    One of kCVPixelFormatType_DisparityFloat16, kCVPixelFormatType_DisparityFloat32, kCVPixelFormatType_DepthFloat16, or kCVPixelFormatType_DepthFloat32.
 */
@property(readonly) OSType depthDataType;

/*!
 @property depthDataMap
 @abstract
    Provides access to the depth data object's internal map.
 
 @discussion
    The depth data map's pixel format can be queried using the depthDataType property.
 */
@property(readonly) __attribute__((NSObject)) CVPixelBufferRef depthDataMap NS_RETURNS_INNER_POINTER;

/*!
 @property depthDataQuality
 @abstract
    Specifies the overall quality of the depth data map's values.
 
 @discussion
    See AVDepthDataQuality documentation for more information.
 */
@property(readonly) AVDepthDataQuality depthDataQuality;

/*!
 @property depthDataFiltered
 @abstract
    Specifies whether the depth data pixel buffer map contains filtered (hole-filled) data.
 
 @discussion
    By setting either AVCaptureDepthDataOutput's filteringEnabled property or AVCapturePhotoSettings' depthDataFiltered property to YES, the resulting depth data are filtered to remove invalid pixel values that may be present due to a variety of factors including low light and lens occlusion. If you've requested depth data filtering, all depth data holes are filled. Note that filtering the depth data makes it more usable for applying effects, but alters the data such that it may no longer be suitable for computer vision tasks. Unfiltered depth maps present missing data as NaN.
 */
@property(readonly, getter=isDepthDataFiltered) BOOL depthDataFiltered;

/*!
 @property depthDataAccuracy
 @abstract
    Specifies the accuracy of the units in the depth data map's values.
 
 @discussion
    See AVDepthDataAccuracy documentation for more information.
 */
@property(readonly) AVDepthDataAccuracy depthDataAccuracy;

/*!
 @property cameraCalibrationData
 @abstract
    The calibration data of the camera with which AVDepthData map's values are aligned.
 
 @discussion
    See AVCameraCalibrationData for more information.
 */
@property(nullable, readonly) AVCameraCalibrationData *cameraCalibrationData;

@end

NS_ASSUME_NONNULL_END
