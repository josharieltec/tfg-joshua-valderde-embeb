#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

const byte receptorAddress[5] = {'R', 'x', 'C', 'T', 'R'};  // Dirección central del receptor

RF24 radio(CE_PIN, CSN_PIN);

#define TRANSMITTER_ID 1  // ⚠️ CAMBIA ESTE NÚMERO (1 a 19) EN CADA TRANSMISOR

char dataToSend[10] = "TxX_000";  // Mensaje base, la 'X' se reemplazará por TRANSMITTER_ID

void setup() {
    Serial.begin(9600);
    Serial.println("Transmisor Iniciando...");
    
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(receptorAddress);  // Se envía al receptor
    radio.stopListening();  // Modo de transmisión

    dataToSend[2] = '0' + TRANSMITTER_ID / 10;  // Decena
    dataToSend[3] = '0' + TRANSMITTER_ID % 10;  // Unidad
}

void loop() {
    static char counter = '0';
    dataToSend[5] = counter;  // Cambia el número en el mensaje
    
    bool sent = radio.write(&dataToSend, sizeof(dataToSend));  // Envía el mensaje
    if (sent) {
        Serial.print("Mensaje enviado: ");
        Serial.println(dataToSend);
    } else {
        Serial.println("Error en el envío");
    }

    counter++;
    if (counter > '9') counter = '0';  // Reiniciar contador después de 9

    delay(random(500, 2000));  // 🕒 Espera aleatoria para evitar colisiones
}
