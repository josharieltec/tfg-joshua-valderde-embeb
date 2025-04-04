#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ------------------ Configuración de pines y constantes ------------------
int currentAnalogInputPin = A1;   // (yellow = Vout)  pin para medir corriente
int calibrationPin = A2;          // (white = Vref)  pin para calibrar offset
float manualOffset = 0.00;        // Offset de calibración manual
float mVperAmpValue = 25.00;      // Sensibilidad del sensor en mV por Amp

float supplyVoltage = 5000;       // Voltaje de referencia para medición (5000mV para Arduino Nano)
int decimalPrecision = 2;         // Número de decimales a mostrar

// Variables de procesamiento de señales
float offsetSampleRead = 0;        
float currentSampleRead = 0;     
float currentLastSample = 0;     
float currentSampleSum = 0;     
float currentSampleCount = 0;     
float currentMean;            
float RMSCurrentMean;             
float FinalRMSCurrent;           

// ------------------- Configuración del módulo nRF24L01 ------------------
RF24 radio(9, 10);  // Pines CE y CSN
const byte direccion[6] = {'R','x','A','A','A'};  // Misma dirección que en el receptor

// Estructura de datos a enviar
struct DataPacket {
    float current1;
};
DataPacket dataToSend;

// ------------------- Configuración ----------------------------------------------
void setup() {              
    Serial.begin(9600);                 // Inicializa el monitor serial

    // Configuración del módulo nRF24L01
    radio.begin();
    radio.setDataRate(RF24_250KBPS);   // Configurar la misma tasa de datos que el receptor
    radio.openWritingPipe(direccion);
    radio.setPALevel(RF24_PA_LOW);
    radio.stopListening();  // Configurado como transmisor
}

// ------------------ Adquisición de datos y transmisión ------------------
void loop() {
    // --------------- ADQUIRIR MUESTRAS ----------------------
    if (micros() >= currentLastSample + 200) { // 5000 muestras por segundo
        currentSampleRead = analogRead(currentAnalogInputPin) - analogRead(calibrationPin); // Lectura y calibración
        currentSampleSum += sq(currentSampleRead);  // Acumula los valores al cuadrado
        currentSampleCount++;  // Incrementa el contador de muestras
        currentLastSample = micros();  // Actualiza el tiempo de la última muestra
    }

    // --------------- PROCESAR Y ENVIAR DATOS CADA 4000 MUESTRAS (0.8s) ---------------
    if (currentSampleCount == 4000) {         
        currentMean = currentSampleSum / currentSampleCount;                
        RMSCurrentMean = sqrt(currentMean);  // Calcula RMS
        FinalRMSCurrent = (((RMSCurrentMean / 1023) * supplyVoltage) / mVperAmpValue) - manualOffset; // Convierte a Amperios

        // Si la corriente medida es menor al 1% de la capacidad del sensor, se asume como 0A 
        if (FinalRMSCurrent <= (625 / mVperAmpValue / 100)) {
            FinalRMSCurrent = 0;
        }

        // ------------------------ MOSTRAR RESULTADOS ------------------------
        Serial.print("Enviado: ");
        Serial.print(FinalRMSCurrent, decimalPrecision);
        Serial.println(" A");

        // ------------------------ TRANSMITIR DATOS ------------------------
        dataToSend.current1 = FinalRMSCurrent;
        
        if (radio.write(&dataToSend, sizeof(dataToSend))) {
            Serial.println("Transmisión exitosa");
        } else {
            Serial.println("Error en la transmisión");
        }

        // Reiniciar valores acumulados para el siguiente ciclo de medición
        currentSampleSum = 0;        
        currentSampleCount = 0;                               
    }
}
