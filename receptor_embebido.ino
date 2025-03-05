#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

const byte receptorAddress[5] = {'R', 'x', 'C', 'T', 'R'};  // Dirección única del receptor

RF24 radio(CE_PIN, CSN_PIN);
char receivedData[10];  // Buffer para almacenar los datos recibidos

void setup() {
    Serial.begin(9600);
    Serial.println("Receptor Iniciando...");
    
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1, receptorAddress);  // Escucha la dirección central
    radio.startListening();  // Comienza a escuchar
}

void loop() {
    if (radio.available()) {
        radio.read(&receivedData, sizeof(receivedData));  // Lee el mensaje
        Serial.print("Mensaje recibido: ");
        Serial.println(receivedData);
    }
}
