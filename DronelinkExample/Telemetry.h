//
//  Telemetry.h
//  DronelinkParrot
//
//  Created by Jim McAndrew on 5/20/20.
//  Copyright © 2020 Dronelink. All rights reserved.
//
struct Telemetry {
    double latitude;
    double longitude;
    double altitude;
    double droneQuatX;
    double droneQuatY;
    double droneQuatZ;
    double droneQuatW;
    double speedNorth;
    double speedEast;
    double speedDown;
    double frameQuatX;
    double frameQuatY;
    double frameQuatZ;
    double frameQuatW;
};

@interface TelemetryHelper : NSObject

+ (struct Telemetry) getTelemetry:(const void* _Nonnull)frame;

@end

