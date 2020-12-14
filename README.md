# DronelinkExample - iOS

## Requirements

- DJISDK key: https://developer.dji.com/
- Mapbox access token: https://account.mapbox.com/access-tokens/create
- Microsoft maps credentials key: https://www.bingmapsportal.com/
- Dronelink environment key: https://www.dronelink.com/
- Dronelink Kernel (dronelink-kernel.js): https://github.com/dronelink/dronelink-kernel-js
- Plans, functions, modes: Export from https://app.dronelink.com/

## Setup

- pod install
- Update bundle identifier to match what was registered with DJI
- Provide DJISDK key in info.plist
- Provide Mapbox public access token in info.plist
- Provide Mapbox secret access token in «USER_HOME»/.netrc (https://docs.mapbox.com/ios/maps/overview/)
- Provide Dronelink environment key and Microsoft maps credentials key in AppDelegate
- Add dronelink-kernel.js to the project by dragging it into the root folder in Xcode (select copy when prompted)

## Author

Dronelink, dev@dronelink.com

## License

DronelinkExample is available under the MIT license. See the LICENSE file for more info.
