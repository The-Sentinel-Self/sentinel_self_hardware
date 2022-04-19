#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <OSCMessage.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
EthernetUDP Udp;
PulseSensorPlayground pulseSensor;

// Choose the MAC address (check box ID)
// byte mac[] = { 0x98, 0x76, 0xB6, 0x11, 0xEC, 0xF8 }; // Heart Rate System 1
// byte mac[] = { 0x98, 0x76, 0xB6, 0x11, 0xEE, 0xD5 }; // Heart Rate System 2
// byte mac[] = { 0x98, 0x76, 0xB6, 0x11, 0xEC, 0x9B }; // Heart Rate System 3
byte mac[] = { 0x98, 0x76, 0xB6, 0x11, 0xEE, 0xD9 }; // Heart Rate System 4

// Set the static IP address to use (check box ID)
// byte ip[] = {192, 168, 0, 201}; // Heart Rate System 1
// byte ip[] = {192, 168, 0, 202}; // Heart Rate System 2
// byte ip[] = {192, 168, 0, 203}; // Heart Rate System 3
byte ip[] = {192, 168, 0, 204}; // Heart Rate System 4

// Receiver
byte outIp[] = {192, 168, 0, 18}; // PC Static IP
const unsigned int outPort = 7539; // PC Static IP Receive Port

// Set the router gateway IP and subnet
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[] = { 255, 255, 255, 0 };




// Heart Rate Variables
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 26;    // A0's GPIO
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int PULSE_FADE = 5;
const int THRESHOLD = 1900;   // Adjust this number to avoid noise when idle
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;






void setup() {
  
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  Udp.begin(8888);
  // Print IP details
  Serial.print("Sentinel is at ");
  Serial.println(Ethernet.localIP());



  
  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);
  
  // Skip the first SAMPLES_PER_SERIAL_SAMPLE in the loop().
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
  
  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    for (;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }

}


void loop() {
  if (pulseSensor.sawNewSample()) {
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      pulseSensor.outputSample();
      if (pulseSensor.sawStartOfBeat()) {
        pulseSensor.outputBeat();
      }
        OSCMessage rate("/sentinel/4/rate");
        rate.add(pulseSensor.getBeatsPerMinute());
        // Serial.println(pulseSensor.getBeatsPerMinute());
        Udp.beginPacket(outIp, outPort);
        rate.send(Udp); // send the bytes to the SLIP stream
        Udp.endPacket(); // mark the end of the OSC Packet
        rate.empty(); // free space occupied by message

        OSCMessage ibi("/sentinel/4/ibi");
        ibi.add(pulseSensor.getInterBeatIntervalMs());
        // Serial.println(pulseSensor.getInterBeatIntervalMs());
        Udp.beginPacket(outIp, outPort);
        ibi.send(Udp); // send the bytes to the SLIP stream
        Udp.endPacket(); // mark the end of the OSC Packet
        ibi.empty(); // free space occupied by message
        
        OSCMessage pulse("/sentinel/4/pulse");
        pulse.add(pulseSensor.getLatestSample());
        // Serial.println(pulseSensor.getLatestSample());
        Udp.beginPacket(outIp, outPort);
        pulse.send(Udp); // send the bytes to the SLIP stream
        Udp.endPacket(); // mark the end of the OSC Packet
        pulse.empty(); // free space occupied by message
    }
  }
}
