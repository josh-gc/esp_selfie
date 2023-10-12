# ESP-IDF Bluetooth Demo
This code is based on the esp_hid_device example script in the esp-idf v4.4 library. The code hard codes in BLE and BT Classic because in testing dual mode was needed to connect to both a Pixel7 (BT Classic) and an iPhone (BLE).   

Also, this code is written to act as a selfie stick so only volume up/down (for triggering the camera shutter) were implemented. Other codes can be added. TODO: Explain in comments how this works can can be done.

# How to Use Example
## Installing
Before project configuration and build, be sure to set the correct chip target using:

```bash
idf.py set-target <chip_name>
```

Configure and build:
```
idf.py -p PORT flash monitor
```
## How to use:
The code will start a handler to capture keystrokes from the user. It will also start broadcasting in BLE and BT Classic as an HID device. Once connected the user can press 'u' or 'd' on the keyboard to adjust the volume on the connected device. If the device is disconnected the esp32 will start broadcasting again.

A few notes/known bugs:
- BUG: The `esp_hidd_send_consumer_value -> esp_hidd_dev_input_set` command does not work with bt classic
- TODO: Change it to only broadcast/stop broadcast with 's' and 'e' commands
- TODO: If there is a connection on BLE or BT Classic, stop broadcasting in the other one


## Hardware Required

* A development board with ESP32 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming
* A phone or computer to connect to the device

### Devices Tested
* Pixel 7 - BT Clasic - Does not work
* iPhone13 - BLE - Works


## Example Output
```
##############################
ESP HID DEVICE DEMO COMMANDS:
You can input these values
s -- Start pairing
e -- End pairing/disconnect
u -- Send volume up
d -- Send volume down
##############################

I (11225) esp_hid_device_main: send_volume_up
E (11225) esp_hid_device_main: HID not connected!
I (12385) esp_hid_device_main: send_volume_down
E (12385) esp_hid_device_main: HID not connected!
W (102765) BT_HCI: hcif disc complete: hdl 0x0, rsn 0x3e
I (103065) ESP_HID_GAP: BLE GAP AUTH SUCCESS
I (103125) ble_hidd_event_callback: CONNECTED
I (111845) esp_hid_device_main: send_volume_up
...
```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/espressif/esp-idf/issues) on GitHub. We will get back to you soon.
