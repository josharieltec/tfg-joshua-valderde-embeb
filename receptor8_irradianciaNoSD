#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Pines
#define CE_PIN   A2 
#define CSN_PIN  9
#define BT_RX    5
#define BT_TX    7

// Comunicación nRF24L01
const byte thisAddress[5] = {'R','x','A','A','A'};
RF24 radio(CE_PIN, CSN_PIN);

struct CurrentData {
    byte senderID;       // 1 o 2
    float currentValue;
};

CurrentData dataReceived;

// Sensor de corriente local
int currentAnalogInputPin = A1;
int calibrationPin = A3;
float manualOffset = 0.4;
float mVperAmpValue = 20.83;
float supplyVoltage = 5000;
int decimalPrecision = 1;
float currentSampleRead = 0;
float currentLastSample = 0;
float currentSampleSum = 0;
float currentSampleCount = 0;
float localCurrent = 0;

// Últimos valores de emisores
float current1 = 0;
float current2 = 0;
bool newDataFrom1 = false;
bool newDataFrom2 = false;

// Bluetooth
SoftwareSerial BTserial(BT_RX, BT_TX);
bool transmitiendo = false;

// Sensor de irradiancia y temperatura
Adafruit_ADS1115 ads;
const float mV_per_watt_m2 = 77.23 / 1000.0;
const float gain_factor = 0.125;
const float Vdd = 5.0;
const float R1 = 1000.0;
float irradiancia = 0;
float temperatura = 0;

void setup() {
    Serial.begin(9600);
    BTserial.begin(9600);

    Serial.println("Receptor esperando datos nuevos de ambos transmisores");
    BTserial.println("Escribe 'BEGIN' para comenzar y 'STOP' para detener");

    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1, thisAddress);
    radio.startListening();

    // Inicializar ADS1115
    if (!ads.begin()) {
        Serial.println("No se encontró el ADS1115. Verifique conexión I2C.");
        while (1);
    }
    ads.setGain(GAIN_ONE);  // ±4.096V, 1 bit = 0.125 mV
}

void loop() {
    recibirComandoBT();
    getData();
    measureCurrent();
    measureIrradianciaTemperatura();

    if (newDataFrom1 && newDataFrom2) {
        storeAndPrintData();
        newDataFrom1 = false;
        newDataFrom2 = false;
    }
}

void recibirComandoBT() {
    if (BTserial.available()) {
        String comando = BTserial.readStringUntil('\n');
        comando.trim();

        if (comando.equalsIgnoreCase("BEGIN")) {
            transmitiendo = true;
            BTserial.println("Transmision iniciada.");
            Serial.println("Transmision iniciada.");
        } else if (comando.equalsIgnoreCase("STOP")) {
            BTserial.println("*JR0G0B0*");
            BTserial.println("*GR0G0B0*");
            BTserial.println("*BR0G0B0*");
            transmitiendo = false;
            BTserial.println("Transmision detenida.");
            Serial.println("Transmision detenida.");
        } else {
            BTserial.println("Comando no reconocido. Usa 'BEGIN' o 'STOP'.");
        }
    }
}

void getData() {
    if (radio.available()) {
        radio.read(&dataReceived, sizeof(dataReceived));

        if (dataReceived.senderID == 1) {
            current1 = dataReceived.currentValue;
            newDataFrom1 = true;
        } else if (dataReceived.senderID == 2) {
            current2 = dataReceived.currentValue;
            newDataFrom2 = true;
        }
    }
}

void measureCurrent() {
    currentSampleSum = 0;
    currentSampleCount = 0;

    while (currentSampleCount < 4000) {
        if (micros() - currentLastSample >= 200) {
            currentSampleRead = analogRead(currentAnalogInputPin) - analogRead(calibrationPin);
            currentSampleSum += sq(currentSampleRead);
            currentSampleCount++;
            currentLastSample = micros();
        }
    }

    float mean = currentSampleSum / currentSampleCount;
    float rms = sqrt(mean);
    float finalCurrent = (((rms / 1023.0) * supplyVoltage) / mVperAmpValue) - manualOffset;

    if (finalCurrent <= (625 / mVperAmpValue / 100)) {
        finalCurrent = 0;
    }

    localCurrent = finalCurrent;
}

void measureIrradianciaTemperatura() {
    int16_t rawIrradiancia = ads.readADC_SingleEnded(1);
    delay(5);
    int16_t rawTemperatura = ads.readADC_SingleEnded(2);

    float mvIrradiancia = rawIrradiancia * gain_factor;
    float mvTemperatura = rawTemperatura * gain_factor;

    irradiancia = mvIrradiancia / mV_per_watt_m2;

    float Vout = mvTemperatura / 1000.0;
    float Rvariable = R1 * ((Vdd / Vout) - 1.0);
    temperatura = (Rvariable - 1000.0) * -0.22;
}

void storeAndPrintData() {
    unsigned long t = millis();

    Serial.print("Tiempo: "); Serial.print(t);
    Serial.print(" | Local: "); Serial.print(localCurrent, decimalPrecision);
    Serial.print(" A | Emisor1: "); Serial.print(current1, decimalPrecision);
    Serial.print(" A | Emisor2: "); Serial.print(current2, decimalPrecision);
    Serial.print(" A | Irradiancia: "); Serial.print(irradiancia, 1);
   // Serial.print(" W/m2 | Temp: "); Serial.print(temperatura, 1);
   // Serial.println(" °C");

    if (transmitiendo) {
                // Encontrar la corriente más alta
    float maxCurrent = max(localCurrent, max(current1, current2));
    float threshold = 0.93 * maxCurrent;

    // Verificar si alguna corriente cae por debajo del 93% de la máxima
    if (localCurrent < threshold) {
    BTserial.println("*JR255G0B0*"); delay(1000); BTserial.println("*JR0G0B0*");
    }

    if (current1 < threshold) {
    BTserial.println("*GR0G255B0*"); delay(1000); BTserial.println("*GR0G0B0*");
    }

    if (current2 < threshold) {
    BTserial.println("*BR0G0B255*"); delay(1000); BTserial.println("*BR0G0B0*");
    }

    }

        // Codigo necesario para graficar

        String mensaje = "*R" +
                         String(localCurrent, decimalPrecision) + "," +
                         String(current1, decimalPrecision) + "," +
                         String(current2, decimalPrecision) + "," +
                         String(irradiancia, 1) + "," +
                         String(temperatura, 1) + "*";
        BTserial.println(mensaje);
                
                //BTserial.println("*I" + String(irradiancia, 1) + "*" );
                //BTserial.println("*T" + String(temperatura, 1) + "*" );                
                BTserial.println("       ");    // 41 digitos
                BTserial.println("NUEVOS DATOS DE CORIENTE:");    // 41 digitos
        

        BTserial.println("Rx = " + String(localCurrent, decimalPrecision) + " A");
        BTserial.println("T1 = " + String(current1, decimalPrecision) + " A ");
        BTserial.println("T2 = " + String(current2, decimalPrecision) + " A");
        BTserial.println("Irr = " + String(irradiancia, 1) + " W/m2" );
       // BTserial.println("Temp = " + String(temperatura, 1) + " C");
        
        



        Serial.println("Enviado BT: " + mensaje);

}
