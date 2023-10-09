/* 
 *  Sentinel Immune Self (OSC via Ethernet)
 *  for use with Adafruit's Feather ESP32 V2,
 *  Adafruit Ethernet Featherwing, and Seeed
 *  Grove Ear-Clip Heart Rate Sensor.
 *
 *  Before uploading to the Feather, check the
 *  MAC, static IP address, and port are correct
 *  for the system, the receiver
 *  IP and port are correct for the system ID and device 
 *  and that the OSC address matches 
 *  the system ID too.
 *
 *  Sam Bilbow (c) 2022
 *  for the Sentinel Immune Self Project
 *  by Sissel Marie Tonn.
 *  www.sambilbow.com 
 *
 *  Modifications by Jonathan Reus 9 Oct 2023
*/




// ----------
// Libraries
// ----------
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <OSCMessage.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
EthernetUDP Udp;
PulseSensorPlayground pulseSensor;




// ----------
// Variables
// ----------
// Reference for system MAC / Receiver Port
// System 1  - 0x98, 0x76, 0xB6, 0x11, 0xEC, 0xF8 - 9001
// System 2  - 0x98, 0x76, 0xB6, 0x11, 0xEE, 0xD5 - 9002
// System 3  - 0x98, 0x76, 0xB6, 0x11, 0xEC, 0x9B - 9003
// System 4  - 0x98, 0x76, 0xB6, 0x11, 0xEE, 0xD9 - 9004

// Static IP address and MAC of the sending Feather
byte mac[] = { 0x98, 0x76, 0xB6, 0x11, 0xEC, 0xF8 };
//byte ip[] = {192, 168, 0, 201}; // for Workstation
byte ip[] = {169, 254, 26, 201}; // for Sissel's laptop


// Static IP address and port of OSC server on the Workstation
//byte receiverIP[] = {169, 254, 31, 247};
//const unsigned int receiverPort = 9001;

// IP address and port of OSC server on Sissel's laptop
byte receiverIP[] = {169, 254, 26, 162};
const unsigned int receiverPort = 9001; // test a different port

bool ETHERNET_ENABLED = true; // is set to false if ethernet connection problems happen

// Set the router gateway IP and subnet [!check if necessary]
//byte gateway[] = { 192, 168, 0, 1 };
//byte subnet[] = { 255, 255, 255, 0 };

// Set the variables for the Heart Rate Sensor
const int OUTPUT_TYPE = SERIAL_PLOTTER; // [!remove?]
const int PULSE_INPUT = 26;     // GPIO for Analog Pin 0
const int PULSE_BLINK = 13;     // Pin 13 is the on-board LED
const int PULSE_FADE = 5;       // Set time (ms) for LED fade
const int THRESHOLD = 1900;     // Threshold to avoid sensor noise when idle [!experiment with this]
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;




// ----------
// Setup - Initialise Serial / Ethernet / UDP connection, begin Pulse Sensor readings
// ----------
void setup() {
   // Open serial communications and wait for port to open before carrying on
   Serial.begin(115200);
   while (!Serial) {
     ;
   }

   delay(1000); // add for stability
   Serial.println("Hello World");

  // Initialise ethernet on CS pin (33 for ESP32 with Adafruit Featherwing Ethernet)
  Ethernet.init(33);

  // Start the Ethernet connection
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    ETHERNET_ENABLED=false;
    for (;;) {
      // Flash the led to show things didn't work
      digitalWrite(PULSE_BLINK, LOW);
      delay(150);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(150);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    ETHERNET_ENABLED=false;
  }

  if(ETHERNET_ENABLED) {
    // Begin UDP from system port 9000
    Udp.begin(9000);

    // Print IP details to serial monitor
    Serial.print("Sentinel is at ");
    Serial.println(Ethernet.localIP());
  }

  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  // Skip the first SAMPLES_PER_SERIAL_SAMPLE in the loop()
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

  // Now that everything is ready, start reading the PulseSensor signal
  if (!pulseSensor.begin()) {
    Serial.println("Cannot initialized Pulse Sensor.");
    for (;;) {
      // Flash the led to show things didn't work
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
}




// ----------
// Loop - Report samples from Heart Rate Sensor, send 3 (heart rate, IBI, pulse) OSC messages to Receiver IP/Port
// ----------
void loop() {
  /* See if a sample is ready from the PulseSensor
    Since USE_INTERRUPTS is false (Arduino ESP32 limitation), this call to sawNewSample()
    will, if enough time has passed, read and process a
    sample (analog voltage) from the PulseSensor. */
  if (pulseSensor.sawNewSample()) {
    // Every so often, send the latest sample to the serial monitor
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      pulseSensor.outputSample();
      // At about the beginning of every heartbeat, report the heart rate and inter-beat-interval.
      if (pulseSensor.sawStartOfBeat()) {
        pulseSensor.outputBeat();
      }

      if(ETHERNET_ENABLED) {
        // Create and send UDP packets using OSC protocol to the Receiver IP/Port using a system specific address.
        OSCMessage rate("/sentinel/1/rate");                                // Create OSC message for heart rate with address
        rate.add((int32_t)pulseSensor.getBeatsPerMinute());                 // Add the latest BPM value from the sensor to the message
        Udp.beginPacket(receiverIP, receiverPort);                          // Begin the UDP packet for the OSC message
        rate.send(Udp);                                                     // Send the bytes to the SLIP stream
        Udp.endPacket();                                                    // Mark the end of the OSC Packet
        rate.empty();                                                       // Free space occupied by message
        //Serial.println((int32_t)pulseSensor.getBeatsPerMinute());           // Print the latest BPM value to serial

        OSCMessage ibi("/sentinel/1/ibi");                                  // Create OSC message for IBI with address
        ibi.add((int32_t)pulseSensor.getInterBeatIntervalMs());             // Add the latest BPM value from the sensor to the message
        Udp.beginPacket(receiverIP, receiverPort);                          // Begin the UDP packet for the OSC message
        ibi.send(Udp);                                                      // Send the bytes to the SLIP stream
        Udp.endPacket();                                                    // Mark the end of the OSC Packet
        ibi.empty();                                                        // Free space occupied by message
        //Serial.println((int32_t)pulseSensor.getInterBeatIntervalMs());      // Print the latest BPM value to serial

        OSCMessage pulse("/sentinel/1/pulse");                              // Create OSC message for pulse with address
        pulse.add((int32_t)pulseSensor.getLatestSample());                  // Add the latest BPM value from the sensor to the message
        Udp.beginPacket(receiverIP, receiverPort);                          // Begin the UDP packet for the OSC message
        pulse.send(Udp);                                                    // Send the bytes to the SLIP stream
        Udp.endPacket();                                                    // Mark the end of the OSC Packet
        pulse.empty();                                                      // Free space occupied by message  
        //Serial.println((int32_t)pulseSensor.getLatestSample());             // Print the latest BPM value to serial
      }
    }
  }
}
