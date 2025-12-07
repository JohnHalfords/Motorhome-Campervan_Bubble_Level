******************************************************************************
* Motorhome/Campervan Bubble Level, by John Halfords, the Netherlands        *
* Java and JSON part is with great thanks to Werner Rothschopf               *
* (https://werner.rothschopf.net/201809_arduino_esp8266_server_client_1.htm) *
******************************************************************************
-- Hardware --
--------------
CPU: ESP8266 Wemos D1 Mini Pro (V3.0.0)
https://nl.aliexpress.com/item/1005006246625522.html

Sensor: MPU6050/GY521
https://nl.aliexpress.com/item/1005007129504945.html

Display: I2C OLED Display Module 0.91 Inch (SSD1306)
https://nl.aliexpress.com/item/1005006365845676.html

-- Connections --
-----------------
See Schematic.png
SDA = D2
SCK or SCL = D1
Internal Led = D4
!! Make sure the MPU and the Display are connected to 3v3 and _NOT_ 5v



