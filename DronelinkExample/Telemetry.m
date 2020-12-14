//
//  Telemetry.m
//  DronelinkExample
//
//  Created by Jim McAndrew on 5/20/20.
//  Copyright © 2020 Dronelink. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Telemetry.h"

#include <pdraw/pdraw_defs.h>
#include <video-metadata/vmeta_frame.h>

@implementation TelemetryHelper

+ (struct Telemetry) getTelemetry:(const void* _Nonnull)frame {
    const struct pdraw_video_frame *pframe = frame;
    
    struct Telemetry telemetry = {0};
    
    if (pframe->metadata.type != VMETA_FRAME_TYPE_V3) {
        return telemetry;
    }
    
    telemetry.latitude = pframe->metadata.v3.base.location.latitude;
    telemetry.longitude = pframe->metadata.v3.base.location.longitude;
    telemetry.altitude = pframe->metadata.v3.base.location.altitude;
    telemetry.droneQuatX = pframe->metadata.v3.base.drone_quat.x;
    telemetry.droneQuatY = pframe->metadata.v3.base.drone_quat.y;
    telemetry.droneQuatZ = pframe->metadata.v3.base.drone_quat.z;
    telemetry.droneQuatW = pframe->metadata.v3.base.drone_quat.w;
    telemetry.speedNorth = pframe->metadata.v3.base.speed.north;
    telemetry.speedEast = pframe->metadata.v3.base.speed.east;
    telemetry.speedDown = pframe->metadata.v3.base.speed.down;
    telemetry.frameQuatX = pframe->metadata.v3.base.frame_quat.x;
    telemetry.frameQuatY = pframe->metadata.v3.base.frame_quat.y;
    telemetry.frameQuatZ = pframe->metadata.v3.base.frame_quat.z;
    telemetry.frameQuatW = pframe->metadata.v3.base.frame_quat.w;
    
    return telemetry;
}

@end
