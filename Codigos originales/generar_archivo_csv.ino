// FUENTE:     https://rydepier.wordpress.com/2015/08/07/using-an-sd-card-reader-to-store-and-retrieve-data-with-arduino/


/*
The circuit:
* SD card attached to SPI bus as follows:
** MOSI - pin 11
** MISO - pin 12
** CLK - pin 13
** CS - pin 10 Uno (53 on Mega)
Based on code by Tom Igoe
*/
//
//the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD
// library functions will not work.

// ----------------------- Blibliotecas incluidas ------------------------------
#include "SD.h"
#include"SPI.h"




// -------------------------- Variables --------------------------
const int CSpin = 10;
String dataString =""; // Almacena datos para ser guardados en SD card
float sensorReading1 = 0.00; // valores de sensores simbolicos para prueba
float sensorReading2 = 0.00; 
float sensorReading3 = 0.00; 
File sensorData;


void setup()
{
// Open serial communications
Serial.begin(9600);
Serial.print("Initializing SD card...");
pinMode(CSpin, OUTPUT);
//
// see if the card is present and can be initialized:
if (!SD.begin(CSpin)) {
Serial.println("Card failed, or not present");
// don't do anything more:
return;
}
Serial.println("card initialized.");
}
//
void loop(){
// build the data string
dataString = String(sensorReading1) + "," + String(sensorReading2) + "," + String(sensorReading3); // convert to CSV
saveData(); // save to SD card
delay(60000); // delay before next write to SD Card, adjust as required
}
//
void saveData(){
if(SD.exists("data.csv")){ // check the card is still there
// now append new data file
sensorData = SD.open("data.csv", FILE_WRITE);
if (sensorData){
sensorData.println(dataString);
sensorData.close(); // close the file
}
}
else{
Serial.println("Error writing to file !");
}
}