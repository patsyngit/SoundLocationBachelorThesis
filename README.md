# SoundLocationBachelorThesis
The purpose of this thesis is to design a device for recording and analysing audio signals.  As part of the work, two devices were created: first one based on Arduino UNO and  second one based on Raspberry Pi 2. The devices communicate via SPI bus with the  MCP3008 analog-to-digital converter and record signals converted from analog to digital  form. The analysed signals come from a circuit consisting of four electret microphones  which were connected to designed amplifiers. The values of the delay between the signals  were determined based on two methods, one of which is based on cross-correlation. Based  on the delay values and the position of the microphones, the position of the sound source  was determined.  
* ArduinoRecordAndSend.ino (Arduino platform):  
 Record and send data from microphones to pc responsible for performing operations via serial communication.  
* MATLABReceiveAndCalc.m (PC platform)  
 Receive data from Arduino, convert it, and perform calculations to determine sound location.  
* RPIDataCalcFull.py (Raspberry Pi platform):  
 Read data from .txt file, convert it, and perform calculations to determine delays between signals from microphones.  
* RPIDataCalcTruncated.py (Raspberry Pi platform):  
 Read data from .txt file, truncate it, and perform calculations to determine delays between signals from microphones.  
* RPIRecordAndWrite.c (Raspberry Pi platform):  
 Record and write data from microphones to .txt file  
* raspberry_12_5k_klasniecie3020.txt - example file of recorded data to use with RPIDataCalcFull.py and RPIDataCalcTruncated.py  
