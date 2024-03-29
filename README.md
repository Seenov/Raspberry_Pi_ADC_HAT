# 6 Channel ADC Hat for Raspberry Pi
Wi-Fi hat with ESP32

For the Raspberry Pi install node-red with version 16 of Nodejs and upload the flow file  "ADC1-esp32c3rev3_2.json" in this root directory. 
The flow file  "ADC1-esp32c3rev3_1.json" works with networks of 512 ip addresses. x.x.0.0 to x.x.1.255 Comment-out line 15 of the "set broadcast ip" node to work with networks of 255 ip addresses.
The list of dependencies for this project are in the file "packages.json"

For Arduino use the sketch in the directory  "RPI_ADC_Hat_analognwifi_rev1"
In the root directory the file "esp32c3setup arduino" show the setting under Tools in the arduino IDE (1.8.16)
The ino file rev2 introduces a self-test feature we use to ensure each unit is functionning properly. DO NOT USE THIS VERSION
The .ino file rev3 is used to test communications with a python script. There is an issue with UDP port usage with this version. It transmitts its messages on port 50450 instead of 5045. It listens on port 5044.

<b>Documentation</b>

<a href="https://seenov.com/2022/03/14/getting_started_6_channel_esp32c3_adc/"><b>Getting </b><b>S</b><b>tarted</b></a>

<a href="https://seenov.com/wp-content/uploads/2022/03/ESP32C3-ADC-hat-specifications.pdf">Specifications</a>

<a href="https://seenov.com/2021/12/15/raspberry-pi-adc-esp32-hat-incredibly-measures-%C2%B112v-with-great-accuracy/">Configuring ADC inputs</a>

<b>Explainer videos Seenov YouTube channel</b>

<a href="https://www.youtube.com/channel/UC92-OhJZzrrUEJLgdM26cIA"> Seenov YouTube Channel</a>
