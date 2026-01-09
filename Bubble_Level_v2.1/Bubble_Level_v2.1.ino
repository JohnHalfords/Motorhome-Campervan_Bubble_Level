// *********************
// [Motorhome/Campervan Bubble Level], by John Halfords, the Netherlands
// This is Version 2.1 (09-01-2026)
// *********************
// What's new:
// -----------
// 1.1 3 XY swap functions that can be true or false depending on how you've mounted the MPU
// 1.2 mDNS so you can use a hostname to browse to instead of an ip-address
// 2.0 Included sensivity
//     ChatGPT made an new Graphical design
//     Put the wwwroot (in progmem) and the personal settings in seperate file
// 2.1 Exchange data with browser with 'WebSocket live stream'
//     ChatGPT helped me with a stable websocket
// *********************
// -- Hardware --
// Display = OLED 0.91
// ESP8266 Wemos D1 Mini Pro
// MPU6050 sensor
// *********************
// -- Connections --
// SDA = D2 // SCK or SCL = D1 // Internal Led = D4 = WiFi led
// Make sure the MPU and the Display are connected to 3v3 and NOT 5v
// *********************
// Extra note: in html you can use ' instead of " to parse html-codes
// in JSON you cannot. That's why the \" is used to use the " without C++ taking it away
// I've used it in sending JSON through the websocket
// *********************
// If you have any ideas, build-up comment or questions, feel free to contact me: halfordsj@gmail.com
// *********************

// Librarys to include
#include <Adafruit_MPU6050.h> // MPU6050 sensor
#include <Adafruit_SSD1306.h> // Display
#include <Adafruit_Sensor.h> // Sensor
#include <ESP8266WiFiMulti.h> // MultiWifi
#include <ESP8266WebServer.h> // Webserver
#include <WebSocketsServer.h> // Websocketserver
#include <ESP8266mDNS.h> // mDNS, so we can use http://camperlevel.local instead of ip-address
// Tip: If your hostname doesn't work on your (Android) phone browser, use 'BonjourBrowser' by Andreas Vogel, Germany.
// This BonjourBrowser app can find your [Motorhome/Campervan Bubble Level], you just click it and you're on the website.

#include "Bubble_Level_html.h" // Make the webserver's wwwroot page in progmem
#include "Bubble_Level_config.h" // Read your personal settings so you don't have to do this in the main program

// Instances
Adafruit_MPU6050 mpu; // MPU6050 sensor
Adafruit_Sensor *mpu_accel; // We don't use *mpu_gyro
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire); // Display
ESP8266WiFiMulti wifiMulti; // MultiWifi
ESP8266WebServer WebServer(80); // Webserver
WebSocketsServer WebSocket(81); // Websocket
MDNSResponder MDNS; // mDNS

// More variables to declare
String ssid, ipadres; // String that holds the ssid and the cliënt ip-adres
float xas, yas; // Levels in floating integer
float swap; // variable used in XY swapping process
float xas_raw, yas_raw; // raw values used in loop to average 50 values

unsigned long lastSend=0; // Websocket
const unsigned long SEND_INTERVAL=50; // ms (20 Hz)  // Websocket

void setup() {

  pinMode(D4, OUTPUT); // Defining D4 as output for WiFi-connect-led

  // MPU things
  mpu.begin();
  mpu.setFilterBandwidth(MPU6050_BAND_10_HZ); // Setting the bandwidth for the mpu
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G); // Setting the range for the mpu
  mpu_accel = mpu.getAccelerometerSensor();
  mpu_accel->printSensorDetails();

  // Dispay things
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Start the display
  display.display(); // Display logo during setup
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);

  Connect_WiFi(); // Sub for connecting to WiFi

  // Webserver, -Socket en mDNS things
  WebServer.on("/",[](){  // We call the landing page 'wwwroot', contents is in progmem
    String page = FPSTR(wwwroot);
    page.replace("@@WS_URL@@", "ws://" + ipadres + ":81/"); // Replace the text "@@WS_URL@@" inside the wwwroot with the now set ip address 
    WebServer.send(200,"text/html",page);
    });
  WebServer.begin(); // Start the webserver
  WebSocket.begin(); // Start the Websocket
  WiFi.setSleep(false); // Prevent the webserver to lose connection, reconnections are slowing down
  MDNS.addService(myhostname, "http", "tcp", 80); // Add the mDNS service
  MDNS.begin(myhostname); // Start the mDNS service
}

void loop() {

  WebServer.handleClient(); // Listen for HTTP requests from clients
  WebSocket.loop(); // Keep the Websocket open
  MDNS.update(); // Keep the mDNS service alive

  sensors_event_t accel; // Acceleration event from the MPU

  // Get 50 values and use the average
  xas = 0; // Set x and y values to 0 before counting to average value
  yas = 0;
    for (int i = 0; i <= 49; i++) { // Get 50 values of x and y and taken the average
      mpu_accel->getEvent(&accel); // get new acceleration values from MPU
      xas_raw = accel.acceleration.x; // Raw value into xas_raw variable
      xas = xas + xas_raw; // Add raw value to xas
      yas_raw = accel.acceleration.y; // Raw value into yas_raw variable
      yas = yas + yas_raw; // Add raw value to yas
      }
  // Above we've gathered 50 values, dividing the sum by 50 will give us the average
  // While we're calculating, we'll substract the calibration values too
  // The calibration values are divided by 'sensivity' because the reading on the website and the display is furtheron multiplied by 'sensivity'
  xas = (xas / 50) - (xas_cal / sensivity);
  yas = (yas / 50) - (yas_cal / sensivity);

  // Swapping XY when swapXY = true
  if (swapXY)
    {
      swap = xas;
      xas = yas;
      yas = swap;
    }
  //  Swap + to - values and vv if swapX or swapY is true
  if (swapX) xas = -xas;
  if (swapY) yas = -yas;
  
  // values multiply by to string
  xas = xas * sensivity;
  yas = yas * sensivity;

   // Display the X and Y value on the display
  display.clearDisplay();
  display.setCursor(0, 0); // (x, y)
  display.print(" X: "); //m/s^2
  display.print(xas);
  display.print("  Y: ");
  display.println(yas);
  
  // Display the SSID and IP-address
  display.setCursor(0, 12); // (x, y)
  display.println(ipadres);
  display.println(ssid);
  display.display();

  // WebSocket handling -> Data to webclient
  unsigned long now=millis();
  if(now-lastSend>=SEND_INTERVAL){
    lastSend=now;
    char json[64];
    snprintf(json,sizeof(json),
      "{\"x\":%.1f,\"y\":%.1f}",xas,yas);

    WebSocket.broadcastTXT(json);
  }
}

// ------------------ END -------------------
//
// ------------------ Voids -------------------

// // // Connect to WiFi
void Connect_WiFi()
{
  // Set SSID and pwd for WiFi (add more lines of you've defined more than 4)
  if (ssid1 != "") wifiMulti.addAP(ssid1, password1);
  if (ssid2 != "") wifiMulti.addAP(ssid2, password2);
  if (ssid3 != "") wifiMulti.addAP(ssid3, password3);
  if (ssid4 != "") wifiMulti.addAP(ssid4, password4);

  digitalWrite(D4, HIGH); // HIGH means wifiled = off

  if (wifiMulti.run() == WL_CONNECTED) // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to one of the networks above
    {
      digitalWrite(D4, LOW); // LOW means wifiled = on
      display.clearDisplay(); // Cls
      display.setCursor(0, 0); // (x, y)
      display.println("Connect to SSID:");
      ssid = WiFi.SSID();
      ipadres = WiFi.localIP().toString();
//      wsUrl = "ws://" + ipadres + ":81/"; // Set the variable to send with the html for connecting with the websocket on ip
      display.println(ssid);
      display.println(ipadres);
      display.display();
      delay(2000);
    } else {
        digitalWrite(D4, HIGH); // HIGH means wifiled = off
        ssid = "No WiFi connection";
        ipadres = "No ip address";
        display.setCursor(0, 0);
        display.println("No WiFi Connection");
        display.display();
        delay(5000);
    }
}
