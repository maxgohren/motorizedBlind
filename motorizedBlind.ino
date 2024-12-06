/*********
  Modified from Rui Santos at https://randomnerdtutorials.com  
*********/

#include <WiFi.h>
#include <Stepper.h>
#include "wifi_config.h" // Include the Wi-Fi credentials

#define STEPS 2038
#define ledPin 2
#define motorPin1 15
#define motorPin2 0
#define motorPin3 4
#define motorPin4 17

// Create stepper object called 'myStepper'
Stepper myStepper = Stepper(STEPS, motorPin1, motorPin2, motorPin3, motorPin4);

// Create an instance of the server on port 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliary variable to store the current output state
String ledState = "off";
String motorState = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(ledPin, OUTPUT);
  // Set outputs to LOW
  digitalWrite(ledPin, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // Set motor speed
  myStepper.setSpeed(5);
}

void loop(){
  WiFiClient client = server.accept();

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /LED/on") >= 0) {
              Serial.println("LED ON");
              ledState = "on";
              digitalWrite(ledPin, HIGH);
            } else if (header.indexOf("GET /LED/off") >= 0) {
              Serial.println("LED OFF");
               ledState = "off";
              digitalWrite(ledPin, LOW);
            } else if (header.indexOf("GET /MOTOR/up") >= 0) {
              Serial.println("Motor up");
              motorState = "moving up";
              myStepper.step(STEPS);
              delay(2000);
              motorState = "off";
            } else if (header.indexOf("GET /MOTOR/up") >= 0) {
              Serial.println("Motor down");
              motorState = "moving down";
              myStepper.step(-STEPS);
              delay(2000);
              motorState = "off";
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for ledPin 
            client.println("<p>LED (Pin 2) - State " + ledState + "</p>");
            // If the ledState is off, it displays the ON button       
            if (ledState == "off") {
              client.println("<p><a href=\"/LED/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/LED/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            // Display current state, and UP and DOWN buttons for stepper motor control 
            client.println("<p>Motor - State " + motorState + "</p>");
            // If the motorState is off, allow the user to move it
            if (motorState == "off") {
              client.println("<p><a href=\"/MOTOR/up\"><button class=\"button\">UP</button></a></p>");
              client.println("<p><a href=\"/MOTOR/down\"><button class=\"button button2\">DOWN</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}