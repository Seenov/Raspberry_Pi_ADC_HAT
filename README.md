# 6 Channel ADC Hat for Raspberry Pi
Wi-Fi hat with ESP32

For the Raspberry Pi install node-red and upload the flow file  "Node-red.json" in this root directory. Make sure you implement the comments in node Set Broadcast ip.

The package.json file
When first started, or a new project created, Node-RED will create an initial package.json file in your user directory, or project directory. This allows you to manage your additional dependencies, and release versions of your project, using standard npm practices. The initial version is 0.0.1 but should be edited according to your project release requirements.
The list of dependencies for this project are in the file "packages.json"

For Arduino use the sketch in the directory  "RPI_ADC_Hat_analognwifi_rev1"I
In the root directory the file "esp32c3setup arduino" show the setting under Tools in the arduino IDE (1.8.16)
There is a problem with the Arduino implementation of th ADC for the ESP32 see issue https://github.com/espressif/arduino-esp32/issues/5502  scrool to
https://github.com/espressif/arduino-esp32/issues/5502#issuecomment-1038255676
The file single_read.c is a modified  example of the esp32-idf where the ADC works perfectly.
A modified version of https://github.com/madhephaestus/ESP32AnalogRead in the folder ESP32C3AnalogRead provides a calibrated voltage output for ADC1. ADC2 still under development. Add the library and replace the 2 files with the files in the above mentionned folder.
