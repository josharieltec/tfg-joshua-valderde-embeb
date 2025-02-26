// Fuente:     https://poweruc.com/faq/how-to-read-current-sensor-with-arduino

// Which pin to measure Current Value (A0 is reserved for LCD Display Shield Button function)
int currentAnalogInputPin = A1;  
// Which pin to calibrate offset middle value
int calibrationPin = A2; 
// Key in value to manually offset the initial value
float manualOffset = 0.00;
// If using “Hall-Effect” Current Transformer, key in value using this formula: mVperAmp = maximum voltage range (in milli volt) / current rating of CT
float mVperAmpValue = 12.5;        
// Analog input pin maximum supply voltage, Arduino Uno or Mega is 5000mV while Arduino Nano or Node MCU is 3300mV
float supplyVoltage = 5000;
/* to read the value of a sample for offset purpose later */
float offsetSampleRead = 0;        
 /* to read the value of a sample including currentOffset1 value*/
float currentSampleRead  = 0;     
  /* to count time for each sample. Technically 1 milli second 1 sample is taken */
float currentLastSample  = 0;     
  /* accumulation of sample readings */
float currentSampleSum   = 0;     
  /* to count number of sample. */
float currentSampleCount = 0;     
    /* to calculate the average value from all samples, in analog values*/
float currentMean ;            
  /* square roof of currentMean, in analog values */
float RMSCurrentMean ;             
   /* the final RMS current reading*/
float FinalRMSCurrent ;           
void setup()                                 /*codes to run once */
{
        Serial.begin(9600);                 /* to display readings in Serial Monitor at 9600 baud rates */
}
void loop()
{
        /* 1- AC & DC Current Measurement */
        if(micros() >= currentLastSample + 200)                     /* every 0.2 milli second taking 1 reading */
          {
           /* read the sample value including offset value*/
           currentSampleRead = analogRead(currentAnalogInputPin)-analogRead(calibrationPin); 
          /* accumulate total analog values for each sample readings*/
           currentSampleSum = currentSampleSum + sq(currentSampleRead) ;
           /* to count and move on to the next following count */
           currentSampleCount = currentSampleCount + 1; 
           /* to reset the time again so that next cycle can start again*/
           currentLastSample = micros(); 
          }
        /* after 4000 count or 800 milli seconds (0.8 second), do this following codes*/
        if(currentSampleCount == 4000) 
          {
            /* average accumulated analog values*/
            currentMean = currentSampleSum/currentSampleCount;
             /* square root of the average value*/
            RMSCurrentMean = sqrt(currentMean);
             /* calculate the final RMS current*/
            FinalRMSCurrent = (((RMSCurrentMean /1023) *supplyVoltage) /mVperAmpValue)- manualOffset;
              /* if the current detected is less than or up to 1%, set current value to 0A*/
            if(FinalRMSCurrent <= (625/mVperAmpValue/100))
            { FinalRMSCurrent =0; }
            Serial.print(” The Current RMS value is: “);
            Serial.print(FinalRMSCurrent,decimalPrecision);
            Serial.println(” A “);
            currentSampleSum =0;                                 /* to reset accumulate sample values for the next cycle */
            currentSampleCount=0;                                 /* to reset number of sample for the next cycle */
          }
}