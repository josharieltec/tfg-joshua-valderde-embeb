// Librerias
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>



// Iniciar modulo nRF24L01
#define CE_PIN   9
#define CSN_PIN 10
const byte thisSlaveAddress[5] = {'R','x','A','A','A'};
RF24 radio(CE_PIN, CSN_PIN);


// Estructura para los datos que se van a recibir
struct CurrentData {
    float current1;  // Corriente del primer emisor
 //   float current2;  // Corriente del segundo emisor
};
CurrentData dataReceived;


// Corriente medida localmente
float localCurrent;  
bool newData = false;

// Pines del sensor de corriente local
int currentAnalogInputPin = A1;  
int calibrationPin = A2;          
float manualOffset = 0.00;        
float mVperAmpValue = 25.00;      
float supplyVoltage = 5000;  
int decimalPrecision = 2;
// Variables para procesamiento de corriente
float offsetSampleRead = 0;        
float currentSampleRead  = 0;     
float currentLastSample  = 0;     
float currentSampleSum   = 0;     
float currentSampleCount = 0;     
float currentMean;            
float RMSCurrentMean;             
float FinalRMSCurrent;            



// Almacenamiento de datos con buffer circular
struct Measurement {
    unsigned long timestamp;
    float localCurrent;
    float receivedCurrent1;
    float receivedCurrent2;
};
Measurement measurements[100];  // Buffer para 100 mediciones
int index = 0;  // Índice del buffer



// Inicializar comunicacion RF
void setup() {
    Serial.begin(9600);
    Serial.println("Receptor nRF24L01 + Medición de corriente");

    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.startListening();
}




// Ciclo que mide la corriente y envia el dato
void loop() {
    getData();
    measureCurrent();
    storeData();
    showData();
}


//
void getData() {
    if (radio.available()) {
        radio.read(&dataReceived, sizeof(dataReceived));
        newData = true;
    }
}


// Proceso para medir la corriente
void measureCurrent() {
    if (micros() >= currentLastSample + 200) {
        currentSampleRead = analogRead(currentAnalogInputPin) - analogRead(calibrationPin);
        currentSampleSum += sq(currentSampleRead);
        currentSampleCount++;
        currentLastSample = micros();
    }

    if (currentSampleCount == 4000) {
        currentMean = currentSampleSum / currentSampleCount;
        RMSCurrentMean = sqrt(currentMean);
        FinalRMSCurrent = (((RMSCurrentMean / 1023) * supplyVoltage) / mVperAmpValue) - manualOffset;

        if (FinalRMSCurrent <= (625 / mVperAmpValue / 100)) {
            FinalRMSCurrent = 0;
        }



        // Esto es diferente en los transmisores
        localCurrent = FinalRMSCurrent;
        currentSampleSum = 0;
        currentSampleCount = 0;
    }
}




// Almacena los datos en la posición actual del buffer circular
void storeData() { 
    measurements[index].timestamp = millis();
    measurements[index].localCurrent = localCurrent;
    measurements[index].receivedCurrent1 = dataReceived.current1;
   // measurements[index].receivedCurrent2 = dataReceived.current2;

    // Avanza el índice circularmente
    index = (index + 1) % 100;  
}

void showData() {
    if (newData) {
        Serial.print("Corriente local: ");
        Serial.print(localCurrent, decimalPrecision);
        Serial.print(" A | Emisor 1: ");
        Serial.print(dataReceived.current1, decimalPrecision);
        Serial.println(" A ");
      //   Serial.print(" A | Emisor 2: ");
     //   Serial.print(dataReceived.current2, decimalPrecision);
     //   Serial.println(" A");
        newData = false;
    }
}