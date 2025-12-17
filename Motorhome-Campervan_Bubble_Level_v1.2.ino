// *********************
// [Motorhome/Campervan Bubble Level], by John Halfords, the Netherlands
// Java and JSON part is with great thanks to Werner Rothschopf (https://werner.rothschopf.net/201809_arduino_esp8266_server_client_1.htm)
// This is Version 1.2 (17-12-2025)
// *********************
// 1.1 What's new: 3 XY swap functions that can be true or false depending on how you've mounted the MPU
// 1.2 What's new: mDNS so you can use a hostname to browse to instead of an ip-address
// What's next? I think sensivity
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
// I've used it in void handleJson() and with building up the strings that will be send bij void handleJson()
// *********************
// If you have any ideas build-up comment or questions, feel free to contact me: halfordsj@gmail.com
// *********************

// Librarys to include
#include <Adafruit_MPU6050.h> // MPU6050 sensor
#include <Adafruit_SSD1306.h> // Display
#include <Adafruit_Sensor.h> // Sensor
#include <ESP8266WiFiMulti.h> // MultiWifi
#include <ESP8266WebServer.h> // Webserver
#include <ESP8266mDNS.h> // mDNS, so we can use http://camperlevel.local instead of ip-address
// If your hostname doesn't work on your (Android) phone browser, use 'BonjourBrowser' by Andreas Vogel, Germany.
// This BonjourBrowser app can find your [Motorhome/Campervan Bubble Level], you just click it and you're on the website.

// Instances
Adafruit_MPU6050 mpu; // MPU6050 sensor
Adafruit_Sensor *mpu_accel, *mpu_gyro;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire); // Display
ESP8266WiFiMulti wifiMulti; // MultiWifi
ESP8266WebServer WebServer(80); // Webserver
MDNSResponder MDNS; // mDNS

// ******************* Personal settings *******************

// Set your own ssid and password in order of importance
// The display will show the one it's connected to, plus the ip-adres for your webbrowser
// Tip: make your phone a hotspot (you don't need data to run this), reset the [Motorhome/Campervan Bubble Level] and browse to the ip-address
// If you plan to use more than 4, make sure you've also added some extra line in void Connect_WiFi()
const char* ssid1 = "Phone";
const char* password1 = "qwertyuiop";
const char* ssid2 = "My home";
const char* password2 = "asdfghjkl";
const char* ssid3 = "";
const char* password3 = "";
const char* ssid4 = "";
const char* password4 = "";

// Set your own hostname
// Working with hostnames (through mDNS) is much easier. Read the advice with the libraries above.
// Now we can reach the webserver with (for example) http://camperlevel.local instead of a possibly changing ip-address
const char* myhostname = "camperlevel";

// Set your own calibration values
// Level your motorhome or campervan for the last time with a 'normal' bubble level, do it very precise
// Mount your [Motorhome/Campervan Bubble Level] in your motorhome or campervan
// Read the values on the display or website and fill them in below
// In void loop() these values will be subtracted so the reading will be 0,0 when leveled
// From now on you can level your motorhome or campervan with your [Motorhome/Campervan Bubble Level]
float xas_cal = 0.00; // for example -1.80;
float yas_cal = 0.00; // for example +0.13;

// Set the following variables to your own situation
// Depends how you mount the MPU6050 sensor
// I advise to mount your complete [Motorhome/Campervan Bubble Level] with the MPU6050 sensor horizontal
// Anyway; setting the following variables can be helpfull
// Golden advice: set the calibration first! or you will get confused...
bool swapXY = false; // if true X and Y values are swapped
bool swapX = false;  // if true the X minus values will be plus and vice versa
bool swapY = false;  // if true the Y minus values will be plus and vice versa

// ******************* END Personal settings *******************

// More variables to declare
String webpage, ssid, ipadres, mac; // String that holds the webpage, ssid, mac address and the cliënt ip-adres
float xas, yas, zas, gxas, gyas, gzas; // Levels in floating integer
float swap; // variable used in XY swapping process
float xas_raw, yas_raw; // raw values used in loop to average 50 values
String xas$, yas$, zas$, gxas$, gyas$, gzas$; // Levels in string
String level; // The string that holds the json data for showing the green dot
int count = 0; // Counter for running indicator
String running[10] = {"\"[o]\"", "\"[o]\"", "\"(x)\"", "\"(x)\"", "\"[0]\"", "\"[0]\"", "\"(0)\"", "\"(0)\"", "\"(#)\"", "\"(#)\"",}; // Running indicator

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
  
  // Webserver things
  WebServer.on("/", wwwroot); // We call the landing page 'wwwroot', contents is in void wwwroot()
  WebServer.on("/j.js", handleJs); // Javascript based on fetch API to update the page, contents is in void handleJs()
  WebServer.on("/json",  handleJson); // Send data in JSON format, contents is in void handleJson()
  WebServer.begin(); // Start the webserver
  MDNS.addService(myhostname, "http", "tcp", 80); // Add the mDNS service
  MDNS.begin(myhostname); // Start the mDNS service
}

void loop() {

  WebServer.handleClient(); // Listen for HTTP requests from clients
  MDNS.update(); // Keep the mDNS service alive

  sensors_event_t accel; // Acceleration from the MPU

  // Get 50 values and use the average
  xas = 0; // Set x and y values to 0 before counting to average value
  yas = 0;
    for (int i = 0; i <= 49; i++) { // Get 50 values of x and y and taken the average
      mpu_accel->getEvent(&accel); // get new acceleration values from MPU
      xas_raw = accel.acceleration.x; // Raw value into xas_raw variable
      xas = xas + xas_raw; // Add raw value to xas
      yas_raw = accel.acceleration.y; // Raw value into yas_raw variable
      yas = yas + yas_raw; // Add raw value to yas
      delay(5); // 5ms * 50 = 250 ms
      }
  // Above we've gathered 50 values, dividing the sum by 50 will give us the average
  // While we're calculating, we'll substract the calibration values too
  // The calibration values are divided by 9 because the reading on the website and the display is furtheron multiplied by 9
  xas = (xas / 50) - (xas_cal / 9);
  yas = (yas / 50) - (yas_cal / 9);

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

  count = count + 1; // Next running character
  if (count  == 10) count = 0; // Start with 1st running character
  
  // values to string
  xas$ = String(xas * 9);
  if (xas >= 0) xas$ = "+" + xas$; //Place a + before the value instead of the minus when positive value
  yas$ = String(yas * 9);
  if (yas >= 0) yas$ = "+" + yas$; //Place a + before the value instead of the minus when positive value
  
  // Display the X and Y value on the display
  display.clearDisplay();
  display.setCursor(0, 0); // (x, y)
  display.print(" X: "); //m/s^2
  display.print(xas$);
  display.print("  Y: ");
  display.println(yas$);
  
  // Display the SSID and IP-address
  display.setCursor(0, 12); // (x, y)
  display.println(ipadres);
  display.println(ssid);
  display.display();

  // Enqoute the strings for X and Y axe for JSON, we do this _after_ displaying them on Oled
  xas$ = "\"" + xas$ + "\"";
  yas$ = "\"" + yas$ + "\"";

  // Make level string for the Green dot that is in the middle when x=0 and y=0
  level = "\"<div style='position:relative;left:";
  level += (xas * 100) + 184; // +184 because that's in the middle of the box
  level += "px;top:";
  level += 196 - (yas * 100); // Subtracted from 196 because that's in the middle of the box
  level += "px;width:16px;height:16px;background-color:#4CFF00;'></div></div>\"";
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
      mac = String(WiFi.macAddress());
      display.println(ssid);
      display.println(ipadres);
      display.display();
      delay(2000);
    } else {
        digitalWrite(D4, HIGH); // HIGH means wifiled = off
        ssid = "No WiFi connection";
        ipadres = "No ip address";
        mac = "Unknown mac address";
        display.setCursor(0, 0);
        display.println("No WiFi Connection");
        display.display();
        delay(5000);
    }
}

// // // Make page wwwroot
void wwwroot()
  {
  // Build the string for the webpage
  webpage = "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'/>"; // to look good on your phone too
  webpage += "<html><head><style>td,th{border: 1px solid #dddddd;text-align:left;padding:3px;}th{background-color:#dddddd;}</style>";
  webpage += "<title>Camper Leveling</title><script src='j.js'></script></head><body>";
  webpage += "<h2><b>Camper Leveling</h2></b>";
  webpage += "<hr><p>WiFi: &nbsp; &nbsp; " + ssid + "<br>";
  webpage += "IP: &nbsp; &nbsp; &nbsp; &nbsp; " + ipadres + "<br>";
  webpage += "MAC: &nbsp; &nbsp;" + mac + "<br><hr>";
  webpage += "<table style= 'width: 100%;'><tr><th>As</th><th>Value</th></tr>";
  // Insert the (empty) values and running string (JSON sends the values)
  webpage += "<tr><td>X-as:</td><td><p><span id='xas'>X-as</p></td></tr>";
  webpage += "<tr><td>Y-as:</td><td><p><span id='yas'>Y-as</p></td></tr></table>";
  webpage += "<p>Running: <span id='running'>Running</p>";
  // Insert the Box and cross
  webpage += "<div style='position:relative;left:2px;top:2px;width:384px;height:440px;border:2px solid grey'>"; //Outside box
  // - // It's easier to draw two boxes then to draw a cross
  webpage += "<div style='position:absolute;left:-1px;top:-1px;width:192px;height:220px;border:1px solid grey'></div>"; // Upper right box
  webpage += "<div style='position:absolute;left:192px;top:220px;width:191px;height:219px;border:1px solid grey'></div>"; // Under left
  // Insert the green dot (JSON sends the complete string + coördinates)
  webpage += "<p><span id='level'>level</p>";
  // Close up
  webpage += "<br></body></html>";
  WebServer.send(200, "text/html", webpage); // Send the webpage to the server
}

// // // Make page with script for handling JSON messages
void handleJs() {
  // Output: a fetch API / JavaScript
  // Note: \n is new line
  webpage = "const url ='json';\n";
  webpage += "function renew(){\n";
  webpage += "                 fetch(url)\n"; // Call the fetch function passing the url of the API as a parameter
  webpage += "                 .then(response => {return response.json();})\n";
  webpage += "                 .then(jo => {\n";
  webpage += "                 for (var i in jo)\n";
  webpage += "                   {if (document.getElementById(i)) document.getElementById(i).innerHTML = jo[i];}\n";
  webpage += "                })\n";
  webpage += "}\n";
  webpage +=  "document.addEventListener('DOMContentLoaded', renew, setInterval(renew, 200));"; // Renew in 200ms
  WebServer.send(200, "text/javascript", webpage); // Send the webpage to the server
}

// // // Make JSON message
void handleJson() {
  // Output: send data to browser as JSON (Build the JSON message/package)
  webpage = "{\"xas\":";
  webpage += xas$;
  webpage += ",\"yas\":";
  webpage += yas$;
  webpage += ",\"running\":";
  webpage += running[count];
  webpage += ",\"level\":";
  webpage += level;
  webpage += "}"; // End of JSON
  WebServer.send(200, "application/json", webpage); // Send the webpage to the server
}
