//
//  ViewController.swift
//  DronelinkExample
//
//  Created by Jim McAndrew on 11/21/19.
//  Copyright © 2019 Dronelink. All rights reserved.
//
import DronelinkCore
import DronelinkCoreUI
import DronelinkParrot
import DronelinkParrotUI
import DronelinkDJI
import DronelinkDJIUI
import UIKit
import os
import GroundSdk

class ViewController: UIViewController {
    private static let log = OSLog(subsystem: "DronelinkExample", category: "ViewController")
    
    @IBAction func onDashboard(_ sender: Any) {
        loadPlan()
        //loadMode()
        //loadFunc()
    }
    
    func loadPlan() {
        guard
            let path = Bundle.main.url(forResource: "test", withExtension: "dronelink")?.path,
            let plan = try? String(contentsOfFile: path)
        else {
            return
        }
        
        present(DashboardWidget.create(microsoftMapCredentialsKey: AppDelegate.mapCredentialsKey), animated: true) {
            do {
                try Dronelink.shared.load(plan: plan, delegate: self) { error in
                    os_log(.error, log: ViewController.log, "Unable to read mission plan: %@", error)
                }
            }
            catch DronelinkError.kernelUnavailable {
                os_log(.error, log: ViewController.log, "Dronelink Kernel Unavailable")
            }
            catch DronelinkError.unregistered {
                os_log(.error, log: ViewController.log, "Dronelink SDK Unregistered")
            }
            catch {
                os_log(.error, log: ViewController.log, "Unknown error!")
            }
        }
    }
    
    func loadFunc() {
        guard
            let path = Bundle.main.url(forResource: "focus-distance-test", withExtension: "dronelink")?.path,
            let _func = try? String(contentsOfFile: path)
        else {
            return
        }
        
        present(DashboardWidget.create(microsoftMapCredentialsKey: AppDelegate.mapCredentialsKey), animated: true) {
            do {
                try Dronelink.shared.load(_func: _func, delegate: self) { error in
                    os_log(.error, log: ViewController.log, "Unable to read function: %@", error)
                }
            }
            catch DronelinkError.kernelUnavailable {
                os_log(.error, log: ViewController.log, "Dronelink Kernel Unavailable")
            }
            catch DronelinkError.unregistered {
                os_log(.error, log: ViewController.log, "Dronelink SDK Unregistered")
            }
            catch {
                os_log(.error, log: ViewController.log, "Unknown error!")
            }
        }
    }
    
    func loadMode() {
        guard
            let path = Bundle.main.url(forResource: "mode", withExtension: "dronelink")?.path,
            let mode = try? String(contentsOfFile: path)
        else {
            return
        }
        
        present(DashboardWidget.create(microsoftMapCredentialsKey: AppDelegate.mapCredentialsKey), animated: true) {
            do {
                try Dronelink.shared.load(mode: mode, delegate: self) { error in
                    os_log(.error, log: ViewController.log, "Unable to read mode: %@", error)
                }
            }
            catch DronelinkError.kernelUnavailable {
                os_log(.error, log: ViewController.log, "Dronelink Kernel Unavailable")
            }
            catch DronelinkError.unregistered {
                os_log(.error, log: ViewController.log, "Dronelink SDK Unregistered")
            }
            catch {
                os_log(.error, log: ViewController.log, "Unknown error!")
            }
        }
    }
}

extension ViewController: MissionExecutorDelegate {
    
    func onMissionEstimating(executor: MissionExecutor) {}
    
    func onMissionEstimated(executor: MissionExecutor, estimate: MissionExecutor.Estimate) {}
    
    func missionEngageDisallowedReasons(executor: MissionExecutor) -> [Kernel.Message]? { nil }
    
    func onMissionEngaging(executor: MissionExecutor) {}
    
    func onMissionEngaged(executor: MissionExecutor, engagement: MissionExecutor.Engagement) {}
    
    func onMissionExecuted(executor: MissionExecutor, engagement: MissionExecutor.Engagement) {}
    
    func onMissionDisengaged(executor: MissionExecutor, engagement: MissionExecutor.Engagement, reason: Kernel.Message) {
        //save mission to back-end using: executor.missionSerializedAsync
        //get asset manifest using: executor.assetManifestSerialized
        //load mission later using Dronelink.shared.load(mission: ...
    }
}

extension ViewController: FuncExecutorDelegate {
    func onFuncInputsChanged(executor: FuncExecutor) {}
    
    func onFuncExecuted(executor: FuncExecutor) {
        guard let mission = executor.executableSerialized else {
            return
        }
        
        do {
            try Dronelink.shared.load(mission: mission, delegate: self) { error in
                os_log(.error, log: ViewController.log, "Unable to read mission: %@", error)
            }
        }
        catch DronelinkError.kernelUnavailable {
            os_log(.error, log: ViewController.log, "Dronelink Kernel Unavailable")
        }
        catch DronelinkError.unregistered {
            os_log(.error, log: ViewController.log, "Dronelink SDK Unregistered")
        }
        catch {
            os_log(.error, log: ViewController.log, "Unknown error!")
        }
    }
}

extension ViewController: ModeExecutorDelegate {
    func modeEngageDisallowedReasons(executor: ModeExecutor) -> [Kernel.Message]? { nil }
    
    func onModeEngaging(executor: ModeExecutor) {}
    
    func onModeEngaged(executor: ModeExecutor, engagement: ModeExecutor.Engagement) {}
    
    func onModeExecuted(executor: ModeExecutor, engagement: ModeExecutor.Engagement) {}
    
    func onModeDisengaged(executor: ModeExecutor, engagement: ModeExecutor.Engagement, reason: Kernel.Message) {}
}
