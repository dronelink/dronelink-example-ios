platform :ios, '13.0'
inhibit_all_warnings!
use_frameworks!

target 'DronelinkExample' do
  pod 'DronelinkCore', :path => '../../private/dronelink-core-ios'
  pod 'DronelinkCoreUI', :path => '../dronelink-core-ui-ios'
  pod 'DronelinkDJI', :path => '../dronelink-dji-ios'
  pod 'DronelinkDJIUI', :path => '../dronelink-dji-ui-ios'
  pod 'DronelinkParrot', :path => '../dronelink-parrot-ios'
  pod 'DronelinkParrotUI', :path => '../dronelink-parrot-ui-ios'
  pod 'DJI-SDK-iOS', '~> 4.14-trial2'
  post_install do |installer|
    installer.pods_project.targets.each do |target|
      target.build_configurations.each do |config|
        config.build_settings['ENABLE_BITCODE'] = 'NO'
      end
    end
  end
end