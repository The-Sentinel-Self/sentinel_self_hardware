#include <SPI.h>
#include <Ethernet.h>

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

// Set the router gateway IP and subnet
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[] = { 255, 255, 255, 0 }; 

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");


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

    // start the server
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
  }


  void loop() {
    // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
      Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 1");  // refresh the page automatically every 5 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            // output the value of each analog input pin
            for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
              int sensorReading = analogRead(analogChannel);
              client.print("analog input ");
              client.print(analogChannel);
              client.print(" is ");
              client.print(sensorReading);
              client.println("<br />");
            }
            client.println("</html>");
            break;
          }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          } else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }


        }
      }
      // give the web browser time to receive the data
      delay(1);
      // close the connection:
      client.stop();
      Serial.println("client disconnected");
    }
  }
