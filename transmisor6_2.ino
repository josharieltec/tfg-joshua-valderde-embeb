// Esta modificado para decirme el tiempo que tarda en hacer el ciclo. eso deberia ser el unico cambio
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Pines nRF24L01
#define CE_PIN  A5
#define CSN_PIN 10

// Dirección del receptor
const byte thisSlaveAddress[5] = {'R','x','A','A','A'};
RF24 radio(CE_PIN, CSN_PIN);

// Estructura de datos con identificador de transmisor
struct CurrentData {
    uint8_t senderID;     // Identificador del transmisor
    float current1;
};
CurrentData dataToSend;

// Pines del sensor de corriente
int currentAnalogInputPin = A1;
int calibrationPin = A3;

// Parámetros de calibración
float manualOffset = 0.4;
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

// Variable para medir duración de ciclo
unsigned long previousTime = 0;

void setup() {
    Serial.begin(9600);
    Serial.println("Transmisor 2 nRF24L01 + Medición de corriente");

    // Inicialización del radio
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(thisSlaveAddress);
    radio.stopListening();

    // Asignar ID de transmisor
    dataToSend.senderID = 2;
}

void loop() {
    unsigned long currentTime = millis();                 // Marca de tiempo al inicio
    unsigned long deltaT = currentTime - previousTime;    // Duración del ciclo anterior
    previousTime = currentTime;                           // Actualiza para siguiente ciclo

    measureCurrent();
    sendData();

    Serial.print("Duración del ciclo: ");
    Serial.print(deltaT / 1000.0, 3);                     // Tiempo en segundos con 3 decimales
    Serial.println(" s");

    delay(1000);  // Asegura que no colisione con otros transmisores
}

// Proceso para medir la corriente
void measureCurrent() {
    currentSampleCount = 0;
    currentSampleSum = 0;

    while (currentSampleCount < 4000) {
        if (micros() - currentLastSample >= 200) {
            currentSampleRead = analogRead(currentAnalogInputPin) - analogRead(calibrationPin);
            currentSampleSum += sq(currentSampleRead);
            currentSampleCount++;
            currentLastSample = micros();
        }
    }

    currentMean = currentSampleSum / currentSampleCount;
    RMSCurrentMean = sqrt(currentMean);
    FinalRMSCurrent = (((RMSCurrentMean / 1023.0) * supplyVoltage) / mVperAmpValue) - manualOffset;

    if (FinalRMSCurrent <= (625 / mVperAmpValue / 100)) {
        FinalRMSCurrent = 0;
    }

    dataToSend.current1 = FinalRMSCurrent;
}

// Proceso para enviar datos
void sendData() {
    radio.write(&dataToSend, sizeof(dataToSend));
    Serial.print("Enviado desde TX2: ");
    Serial.print(dataToSend.current1, decimalPrecision);
    Serial.println(" A");
}
