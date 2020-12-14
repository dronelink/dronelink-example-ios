//
//  AppDelegate.swift
//  DronelinkExample
//
//  Created by Jim McAndrew on 11/21/19.
//  Copyright © 2019 Dronelink. All rights reserved.
//
import DronelinkCore
import DronelinkDJI
import DronelinkParrot
import UIKit
import os
import GroundSdk

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {
    private let log = OSLog(subsystem: "DronelinkExample", category: "AppDelegate")
    
    internal static let mapCredentialsKey = "INSERT YOUR CREDENTIALS KEY HERE"

    var window: UIWindow?
    
    var assetManifest: AssetManifest?
    var assetIndex: Int?
    
    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        DronelinkParrot.telemetryProvider = TelemetryProvider.shared
        Dronelink.shared.register(environmentKey: "INSERT YOUR ENVIRONMENT KEY HERE")
        Dronelink.shared.add(droneSessionManager: DJIDroneSessionManager())
        Dronelink.shared.add(droneSessionManager: ParrotDroneSessionManager())
        do {
            //use Dronelink.KernelVersionTarget to see the minimum compatible kernel version that the current core supports
            try Dronelink.shared.install(kernel: Bundle.main.url(forResource: "dronelink-kernel", withExtension: "js")!)
            assetManifest = try? Dronelink.shared.createAssetManifest(id: "example", tags: ["tag1", "tag2"])
            assetIndex = assetManifest?.addAsset(key: "key", descriptors: Kernel.Descriptors(name: "name", description: "description", tags: ["tag1", "tag2"]))
        }
        catch DronelinkError.kernelInvalid {
            os_log(.error, log: self.log, "Dronelink Kernel Invalid")
        }
        catch DronelinkError.kernelIncompatible {
            os_log(.error, log: self.log, "Dronelink Kernel Incompatible")
        }
        catch {
            os_log(.error, log: self.log, "Unknown error!")
        }
        return true
    }
}

extension AppDelegate: DroneSessionManagerDelegate {
    func onOpened(session: DroneSession) {
        session.add(delegate: self)
    }
    
    func onClosed(session: DroneSession) {
        Dronelink.shared.announce(message: "\(session.name ?? "drone") disconnected")
    }
}

extension AppDelegate: DroneSessionDelegate {
    func onInitialized(session: DroneSession) {
        Dronelink.shared.announce(message: "\(session.name ?? "drone") connected")
    }
    
    func onDroneSessionManagerAdded(manager: DroneSessionManager) {
        manager.add(delegate: self)
    }

    func onLocated(session: DroneSession) {}

    func onMotorsChanged(session: DroneSession, value: Bool) {}

    func onCommandExecuted(session: DroneSession, command: KernelCommand) {}

    func onCommandFinished(session: DroneSession, command: KernelCommand, error: Error?) {}
    
    func onCameraFileGenerated(session: DroneSession, file: CameraFile) {
        assetManifest?.addCameraFile(assetIndex: assetIndex ?? 0, cameraFile: file)
        //assetManifest?.serialized to get the manually tracked asset manifest json
    }
}

class TelemetryProvider: ParrotTelemetryProvider, DroneSessionManagerDelegate, YuvSinkListener {
    public static let shared = TelemetryProvider()
    
    public var telemetry: DatedValue<ParrotTelemetry>?
    private var streamServerRef: Ref<StreamServer>?
    private var liveStreamRef: Ref<CameraLive>?
    private var session: DroneSession?
    private var takeoffAltitude: Double?
    
    func onOpened(session: DroneSession) {
        self.session = session
        if let adapter = session.drone as? ParrotDroneAdapter {
            streamServerRef = adapter.drone.getPeripheral(Peripherals.streamServer) { streamServer in
                if let streamServer = streamServer {
                    streamServer.enabled = true
                    self.liveStreamRef = streamServer.live { liveStream in
                        if let liveStream = liveStream {
                            _ = liveStream.play()
                            _ = liveStream.openYuvSink(queue: DispatchQueue.main, listener: self)
                        }
                    }
                }
            }
        }
    }
    
    func onClosed(session: DroneSession) {
        self.session = nil
        liveStreamRef = nil
        streamServerRef = nil
        takeoffAltitude = nil
    }
    
    public func didStart(sink: StreamSink) {
        takeoffAltitude = nil
    }
    
    public func didStop(sink: StreamSink) {
        telemetry = nil
    }

    public func frameReady(sink: StreamSink, frame: SdkCoreFrame) {
        if let pdrawFrame = frame.pdrawFrame {
            let telemetry = TelemetryHelper.getTelemetry(pdrawFrame)
            
            if takeoffAltitude == nil || !(self.session?.state?.value.isFlying ?? false) {
                takeoffAltitude = telemetry.altitude
            }
            
            self.telemetry = DatedValue(value: ParrotTelemetry(
                latitude: telemetry.latitude,
                longitude: telemetry.longitude,
                altitude: telemetry.altitude - takeoffAltitude!,
                takeoffAltitude: takeoffAltitude!,
                droneQuatX: telemetry.droneQuatX,
                droneQuatY: telemetry.droneQuatY,
                droneQuatZ: telemetry.droneQuatZ,
                droneQuatW: telemetry.droneQuatW,
                speedNorth: telemetry.speedNorth,
                speedEast: telemetry.speedEast,
                speedDown: telemetry.speedDown,
                frameQuatX: telemetry.frameQuatX,
                frameQuatY: telemetry.frameQuatY,
                frameQuatZ: telemetry.frameQuatZ,
                frameQuatW: telemetry.frameQuatW))
        }
    }
}
