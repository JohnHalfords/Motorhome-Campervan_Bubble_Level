// ******************* Personal settings *******************
//
// Only set personal settings here!
// Not in the main file, unless you know what you are doing...
//
// Set your own ssid and password in order of importance
// The display will show the one it's connected to, plus the ip-adres for your webbrowser
// Tip: make your phone a hotspot (you don't need data to run this), reset the [Motorhome/Campervan Bubble Level] and browse to the ip-address
// If you plan to use more than 4, make sure you've also added some extra line in void Connect_WiFi()
const char* ssid1 = "Phone";
const char* password1 = "asdfghjkl";
const char* ssid2 = "Home";
const char* password2 = "qwertyuiop";
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
// In the program these values will be subtracted so the reading will be 0,0 when leveled
// From now on you can level your motorhome or campervan with your [Motorhome/Campervan Bubble Level]
float xas_cal = +3.39; // X value to add in your situation, for example +3.4 or +3.39
float yas_cal = -2.14; // Y value to add in your situation, for example -2.1 or -2.14

// Set you own sensivity
// The default output value from the MPU6050 is about -10 to 0 to 10 in 180 degrees turn
// This gets multiplied by the following variable
// If you use 9 the output will be approximately equal to degrees
int sensivity = 9;

// Set the following variables to your own situation
// Depends how you mount the MPU6050 sensor
// I advise to mount your complete [Motorhome/Campervan Bubble Level] with the MPU6050 sensor horizontal
// Anyway; setting the following variables can be helpfull
// Golden advice: set the calibration first! or you will get confused...
bool swapXY = false; // if true X and Y values are swapped
bool swapX = false;  // if true the X minus values will be plus and vice versa
bool swapY = false;  // if true the Y minus values will be plus and vice versa

// ******************* END Personal settings *******************