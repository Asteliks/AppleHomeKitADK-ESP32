# Simple weather station

With this code a user can check the temperature and humidity on his or her Apple device via the [Home app](https://www.apple.com/ios/home/).
## For the sketch to work you need to:
1.	Compile the code (this creates the full sdkconfig file)
2.	Enter your WIFI SSID and password (SDK Configuration (sdkconfig) -> Example Configuration)
3.	A DHT22 sensor is connected to PIN 4.
4.	Compile and upload to target ESP32 / ESP32S2
5.	Upload the Setup Code, look [Change The Setup Code]( https://github.com/espressif/esp-apple-homekit-adk#change-the-setup-code)
6.	[Add]( https://github.com/espressif/esp-apple-homekit-adk#add-acccessory-in-the-home-app) accessory in the Home app
## Should everything work this will be the result
![alt text]( https://github.com/Asteliks/AppleHomeKitADK-ESP32/blob/master/weatherStation/pictures/IMG_3935.JPG)
![alt text]( https://github.com/Asteliks/AppleHomeKitADK-ESP32/blob/master/weatherStation/pictures/IMG_3934.PNG)
![alt text]( https://github.com/Asteliks/AppleHomeKitADK-ESP32/blob/master/weatherStation/pictures/IMG_3936.PNG)
![alt text]( https://github.com/Asteliks/AppleHomeKitADK-ESP32/blob/master/weatherStation/pictures/IMG_3937.PNG)
