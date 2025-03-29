// Fuente:     https://poweruc.com/faq/how-to-read-current-sensor-with-arduino


// --------------DEFINICION DE PINES Y CONSTANTES-------------------------------------
int currentAnalogInputPin = A1;   // (yellow = Vout)  pin para medir corriente (A0 is reserved for LCD Display Shield Button function)
int calibrationPin = A2;          //(white = Vref)  pin para calibrar offset middle value
float manualOffset = 0.00;        // (black = 0V = ground) valor para cambio manual del offset the initial value
float mVperAmpValue = 25.00;       //  If using “Hall-Effect” Current Transformer, key in value using this formula: mVperAmp = maximum voltage range (in milli volt) / current rating of CT
// maximum voltage range = 0.625V
// current rating of CT = 30A
// mVperAmpValue = 625/30 = 20.83 mV

// 2500 / 200 = 12.5
// 2500 / 30 = 83,30  (me da valores muy malos)




float supplyVoltage = 5000;  // (red = 5V o 3.3V) 
// maximum supply voltage, Arduino Uno or Mega is 5000mV while Arduino Nano or Node MCU is 3300mV

int decimalPrecision = 2; // Cantidad de decimales a mostrar, ESTO NO ESTABA ANTES
/*

We connect RED to 5V input, BLACK to ground, YELLOW to analog pin A1, 
and WHITE to analog pin A2. 

It is highly recommended to use a 12V power adapter to power the Arduino Uno 
and the sensor. 

Using a 5V power supply over USB will have an initial OFFSET VALUE, 
you may need to manually add the offset value when uploading the code.

*/


// ------------------VARIABLES PARA EL PROCESAMIENTO DE SENALES------------------------------------------
//Estas variables se utilizan para almacenar muestras de corriente, calcular promedios y determinar el valor RMS de la corriente medida.

float offsetSampleRead = 0;        
float currentSampleRead  = 0;     
float currentLastSample  = 0;     
float currentSampleSum   = 0;     
float currentSampleCount = 0;     
float currentMean ;            
float RMSCurrentMean ;             
float FinalRMSCurrent ;           






// -------------------CONFIGURACION----------------------------------------------
void setup()                                 // codes to run once 
{
        Serial.begin(9600);                 // Serial Monitor at 9600 baud rates
}







//----------------------------  ADQUIRIR MUESTRAS  -----------------------------------------------------
// Proceso para medir la corriente
void loop()
{
        if(micros() >= currentLastSample + 200)              // toma una lectura cada 0.2ms, son 5000 muestras por segundo
          {
           currentSampleRead = analogRead(currentAnalogInputPin)-analogRead(calibrationPin); // Se lee el valor del sensor en A1 y se le resta la referencia en A2 para calibrar el offset
           currentSampleSum = currentSampleSum + sq(currentSampleRead) ;                     //Se acumulan los valores al cuadrado (sq(x)) para luego calcular la media cuadrática.
           currentSampleCount = currentSampleCount + 1;                                      // incrementa el conteo de muestras
           currentLastSample = micros();                                                     // permite iniciar el siguiente conteo de ciclos
          }


        // --------------- PROCESAR DATOS CADA 4000 MUESTRAS O 0.8s --------------------------------
        if(currentSampleCount == 4000)         
          {
            currentMean = currentSampleSum/currentSampleCount;                
            RMSCurrentMean = sqrt(currentMean);                //  Se calcula la media de los valores al cuadrado y luego se obtiene su raíz cuadrada (RMS).
            FinalRMSCurrent = (((RMSCurrentMean /1023) *supplyVoltage) /mVperAmpValue)- manualOffset;               /* calculate the final RMS current*/
            // Se convierte la lectura del ADC (0-1023) a voltaje.
            // Se divide por mVperAmpValue para obtener la corriente en Amperios.
            // Se aplica manualOffset para corregir cualquier error de calibración.


            // Si la corriente medida es menor al 1% de la capacidad del sensor, se asume que es 0A 
            if(FinalRMSCurrent <= (625/mVperAmpValue/100))
            { FinalRMSCurrent =0; }
            
            
            
            // ------------------------ MOSTRAR RESULTADOS ----------------------------------------------
            Serial.print(" La corriente RMS es:  ");      // NOTA: LAS COMILLAS PUEDEN ESTAR MAL ESCRITAS
            Serial.print(FinalRMSCurrent,decimalPrecision);
            Serial.println(" A ");
            currentSampleSum =0;        // Se reinician los valores acumulados para el siguiente ciclo de medicion
            currentSampleCount=0;                               
          }
}