
// Librerias
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


// Iniciar modulo nRF24L01
#define CE_PIN   9
#define CSN_PIN 10
const byte thisSlaveAddress[5] = {'R','x','A','A','A'};
RF24 radio(CE_PIN, CSN_PIN);


// Estructura para los datos que se van a  enviar
struct CurrentData {
    float current1;
//    float current2;    // LO AGREGO SI VOY A ENVIAR VARIAS CORRIENTES
};
CurrentData dataToSend;






// Pines del sensor de corriente
int currentAnalogInputPin = A1;  
int calibrationPin = A2;          
float manualOffset = 0.00;        
float mVperAmpValue = 25.00;  // Lo puedo cambiar para mejor presicion    
float supplyVoltage = 5000;  
int decimalPrecision = 2;
// Variables para medicion de corriente
float offsetSampleRead = 0;        
float currentSampleRead  = 0;     
float currentLastSample  = 0;     
float currentSampleSum   = 0;     
float currentSampleCount = 0;     
float currentMean;            
float RMSCurrentMean;             
float FinalRMSCurrent;            















// Inicializar comunicacion RF
void setup() {
    Serial.begin(9600);
    Serial.println("Emisor nRF24L01 + Medición de corriente");

    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(thisSlaveAddress);    // ESTAS DOS ULTIMAS LINEAS SON DISTINTAS AL OTRO TRANSMISOR
    radio.stopListening();
}




// Ciclo que mide la corriente y envia el dato
void loop() {
    measureCurrent();
    sendData();
    delay(1000);
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

        dataToSend.current1 = FinalRMSCurrent;
     //   dataToSend.current2 = 0;  // CREO QUE ESTO NO ESTA HACIENDO NADA, LO PUEDO ELIMINAR


        currentSampleSum = 0;
        currentSampleCount = 0;
    }
}




// Proceso para enviar datos
void sendData() {
    radio.write(&dataToSend, sizeof(dataToSend));
    Serial.print("Enviado: ");
    Serial.print(dataToSend.current1, decimalPrecision);
    Serial.println(" A");
}