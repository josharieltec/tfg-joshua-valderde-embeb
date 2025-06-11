// Librerías
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Pines nRF24L01
#define CE_PIN  A5
#define CSN_PIN 10    // EN LOS NANO ES 9, AQUI DEJO 10 POR COMODIDAD!!!

// Dirección del receptor
const byte thisSlaveAddress[5] = {'R','x','A','A','A'};
RF24 radio(CE_PIN, CSN_PIN);

// Estructura de datos con identificador de transmisor
struct CurrentData {
    uint8_t senderID;     // Nuevo campo para identificar el transmisor
    float current1;
};
CurrentData dataToSend;

// Pines del sensor de corriente
int currentAnalogInputPin = A1;
int calibrationPin = A3;

// Parámetros de calibración
float manualOffset = 0.2;
float mVperAmpValue = 20.83;
float supplyVoltage = 5000;
int decimalPrecision = 1;

// Variables de cálculo de corriente
float offsetSampleRead = 0;
float currentSampleRead  = 0;
float currentLastSample  = 0;
float currentSampleSum   = 0;
float currentSampleCount = 0;
float currentMean;
float RMSCurrentMean;
float FinalRMSCurrent;

void setup() {
    Serial.begin(9600);
    Serial.println("Transmisor 1 nRF24L01 + Medición de corriente");

    // Inicialización del radio
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(thisSlaveAddress);
    radio.stopListening();

    // Asignar ID de transmisor
    dataToSend.senderID = 1;  // Este transmisor es el número 1
}

void loop() {
    measureCurrent();
    sendData();
    delay(1000);  // Asegura que no colisione con otros transmisores
}

// Proceso para medir la corriente (versión con while bloqueante)
void measureCurrent() {
    currentSampleCount = 0;
    currentSampleSum = 0;

    while (currentSampleCount < 4000) {
        if (micros() - currentLastSample >= 200) {
            currentSampleRead = analogRead(currentAnalogInputPin) - analogRead(calibrationPin);
            currentSampleSum += sq(currentSampleRead);
            currentSampleCount++;
            currentLastSample = micros();

            // DEBUG opcional:
            // Serial.println(currentSampleCount);
        }
    }

    currentMean = currentSampleSum / currentSampleCount;
    RMSCurrentMean = sqrt(currentMean);
    FinalRMSCurrent = (((RMSCurrentMean / 1023.0) * supplyVoltage) / mVperAmpValue) - manualOffset;

    // Eliminar ruido por debajo de cierto umbral
    if (FinalRMSCurrent <= (625 / mVperAmpValue / 100)) {
        FinalRMSCurrent = 0;
    }

    dataToSend.current1 = FinalRMSCurrent;
}

// Proceso para enviar datos
void sendData() {
    radio.write(&dataToSend, sizeof(dataToSend));
    Serial.print("Enviado desde TX1: ");
    Serial.print(dataToSend.current1, decimalPrecision);
    Serial.println(" A");
}
