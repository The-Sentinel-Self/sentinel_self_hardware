# Sentinel Self (Sensor Implementation)
The Arduino sketch contained in this repository allows for the interfacing of a heart rate sensor with the corresponding Unity project found in the [main Sentinel project](https://github.com/The-Sentinel-Self/TheSentinelSelf). The sketch extracts real-time data from the sensor, wraps it in an OSC message and sends it to a port on a specific IP address that is specified in the script.

## Reference
### List of used hardware
- [Adafruit Feather ESP32 V2](https://learn.adafruit.com/adafruit-esp32-feather-v2?view=all) - microcontroller
- [Adafruit Feather Tripler](https://www.adafruit.com/product/3417) - prototyping board that allows easy pin access
- [Adafruit Ethernet Featherwing](https://learn.adafruit.com/adafruit-wiz5500-wiznet-ethernet-featherwing/overview) - enables ethernet networking
- [Seeed Studio Ear Clip Heart Rate Monitor](https://www.seeedstudio.com/Grove-Ear-clip-Heart-Rate-Sensor.html) - the heart rate sensor
- Ethernet cable x1
- USB C <-> mains power or -> USB cable (USB <-> USB needed for uploading sketch)

### List of used software and libraries
- [Arduino IDE](https://www.arduino.cc/en/software) - software used to upload/flash Arduino sketches
- [ESP Board Support](https://github.com/espressif/arduino-esp32) - hardware libraries that allows Arduino to "talk to" ESP32 boards
- [OSC](https://www.arduino.cc/reference/en/libraries/osc/) by Adrian Freeed - library that allows us to send OSC messages from the microcontroller to the host workstation
- [PulseSensorPlayground](https://www.arduino.cc/reference/en/libraries/pulsesensor-playground/) by Joel Murphy, Yury Gitman, and Brad Needham - library that handles converting heart rate signal into legibile data (pulse, interval, rate)

### Pinout
- Ethernet Featherwing connected via tripler
- Seeed Studio Sensor connected to A0, 3V3, and GND 
- This is a manually soldered connection on the central board of the tripler.

## Usage Instructions
### Setting up the software (Windows)
1. Download [Legacy Arduino IDE (v1)](https://www.arduino.cc/en/software)
2. Open the Arduino IDE, and navigate to the Libraries Manager `(Tools > Manage Libraries)`
3. Search for and install the [OSC 1.3.7](https://www.arduino.cc/reference/en/libraries/osc/) and [PulseSensorPlayground 1.6.1](https://www.arduino.cc/reference/en/libraries/pulsesensor-playground/) libraries
4. In File > Preferences, set Additional Boards Manager URL to `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json`
5. Open Tools > Boards > Boards Manager and install the latest stable release of `esp32`
  

### Setting up the hardware (Windows)
1. Ensure that the ESP32 V2 is plugged into your computer via USB (let any drivers automatically install)
2. Find the COM port that the ESP32 V2 is attached to:
- Right-click on the Windows logo on the Start Bar and left-click on Device Manager.
- Navigate to `Ports (COM & LPT)` and note down the COM port, e.g. `COM4` of the device labelled `Silicon Labs CP210x USB to UART Bridge (COM-)` or similar.
3. Plug the ethernet cable into the workstation from the ESP32 ethernet board.
4. Find the IP address of the workstation on the ethernet interface
- Use Windows search to open Command Prompt by typing `cmd`
- Enter `arp -a` to get a list of interface addresses and accessible IPs
- The ethernet interface is likely to be using an APIPA style IP beginning with `169...`
- Note down this address in its entirety e.g. `169.254.x.x`


### Uploading the sketch to the ESP32 V2
1. Open the [sketch](https://github.com/The-Sentinel-Self/sentinel_self_hardware/blob/main/sentinel_self_arduino_ethernet/sentinel_self_arduino_ethernet.ino) in the Arduino IDE, and set up the microcontroller communication
- Navigate to `Tools > Port` and select the COM port you noted down from `Device Manager` earlier
- Navigate to `Tools > Board` and select `ESP32 Arduino > Adafruit Feather ESP32 V2`

2. Scroll down to the `Variables` section and change the following:
- Set `line 52` `receiverIP[]` to the workstation IP you noted down earlier, example format: changes from `169.254.31.247` to `{169, 254, 31, 247}`
- Set `line 49` `ip[]` to the workstation IP you noted down earlier but change the last integer to something unique, e.g. `receiverIP[]` was `{169, 254, 31, 247}`, `ip[]` is `{169, 254, 31, 201}`
- Set `line 48` `mac[]` to the MAC address found on the sticker on the side of the enclosure, the format can be found on `lines 42-45` for easy copy-pasting


1. Verify the code (ctrl-r).
2. Upload the code (ctrl-u). If everything has worked, you will see esptool.py load in the console, and establish connection via COM port, and upload the sketch.


### Connecting to Unity
- Close the Arduino IDE
- Close the project if you have it open
- If setting up on a new workstation, delete the firewall rule for the Unity editor being used, so that the next time Unity is opened it prompts you to either allow or disallow network connections (they need to be allowed to access incoming OSC sensor data)
- Restart your computer
- Plug the sensor box into mains power, and into the ethernet switch or router
- Ensure the workstation is connected to the same ethernet switch or router
- Check that the sensor box has been assigned an IP on the ethernet switch or router by opening Command Prompt, and entering `arp -a`. The interface checked earlier should now have an entry under it corresponding to the variable you set for `ip[]` in 'Uploading the Sketch' step 2, e.g. ending `201`, it should also be listed as `dynamic`
- Open the Unity project and navigate to the Heart Rate Manager GameObject
- Enter Play Mode
- Check if sensor data is coming through via OSC on the Heart Rate Manager Game Object

## Troubleshooting (Restart after each step as needed)
### Unity
- Ensure HeartRateManger GameObject is active, its children SentinelReceives are active too, as well as the OSCReceiver component.
- Ensure the Unity editor is not having its network connection blocked by the firewall. 
  - Open firewall settings and delete incoming rules for the Unity editor in question
  - Restart Unity and allow all connections on all networks (private and public)

### Arduino
- COM Port is not available
  - Double check that you have selected the right COM port in `Tools > Port` and that it corresponds with the COM port found in  Device Manager. 
  - Use a different USB port, and repeat 'Setting up Hardware' step 2
- I suspect OSC data is not being received on workstation
  - Use a tool like Protokol to monitor the receiver port (9001 is default but is set by `line 53`). If no data is incoming then it is an issue with IP addresses, double check 'Setting up Hardware' steps 4.1 or 4.2. 
  - Try a different ethernet port

### Windows Firewall
- Open Windows Firewall > Inbound Rules
- Action > New Rule
1. Port
2. TCP > Specific > 2001
3. Allow the connection
4. Domain, Private and Public
5. Name > Sentinel
  
### Windows Network Type
- The network interface has sometimes not worked if it is set as a Public Network.
- This can be changed for a non-internet connection (as in our case) by using Windows Powershell
- With the Sentinel system set up and connected: Windows > Powershell > Run as administrator
- Enter `get-netconnectionprofile`
- Note down the "name" of the Ethernet connection, it is most likely "Unidentified network"
- Enter `set-netconnectionprofile -name "Unidentified network" -networkcategory private` substituting where necesarry
- Restart