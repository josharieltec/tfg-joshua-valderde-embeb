//Fuente: https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
//Library: TMRh20/RF24, https://github.com/tmrh20/RF24/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>



RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";   // Cualquier 5 letras funcionan

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);       // Se puede aumentar la potencia si necesito mayor distancia
  // En potencias mas altas es recomendable utilizar un Bypass Capacitor entre Ground y 3.3V para obtener voltaje mas estable
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
}