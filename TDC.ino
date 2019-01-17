//Developed by Vikas Agaradahalli
//To read the TDC values and compute the TOF
//readRegister and writeRegister functions to read and write from registers
//double2s functions converts double values to string
//In this code "overflow" and "mask" registers' valus are default
//The sequence of measurement is as described in section 8.4.6 on page no.20 of the datasheet
//Use level shifter or power arduino using 3.3V
//The IC uses onboard clock (8Mhz) on evaluation kit

#include <SPI.h>
#include <print.h>

int CLK = 3; //IC clock if onboard clock not used
int STR = 9; // START and STOP signals for the TDC
//int STP = 6;
int en = 8; //IC enable
int CS = 10; // SPI chip select
int t = 0; 
int p = 0;
const byte READ = 0b10111111;
const byte WRITE = 0b01000000;
double CLK_PERIOD = 125 * pow(10,-9); // Change if IC clock frequency changed
int CALIB_PERIODS = 10;
int TRIG = 7; //IC TRIG pin
int INT = 6; //IC active low interrupt pin, indicates measurement complete 
int addr = 0; //keeps track of EEPROM address 
// MISO(12), MOSI(11) & SCLK(13) should be connected to IC accordingly 

void setup()
 {
   Serial.begin(9600);

   pinMode(TRIG,INPUT);
   pinMode(INT,INPUT);
 //STR and STP signals are same
   pinMode(STR,OUTPUT);
  //  pinMode(STP,OUTPUT);
   delay(500);

   // low to high transition of the Enable
   pinMode(en,OUTPUT);
   digitalWrite(en,LOW);
   delay(2);
   digitalWrite(en,HIGH);
   
   // begin SPI and set clk to 20MHz and Mode to SPI_MODE1
   SPI.begin();
   //SPISettings(8000000, MSBFIRST, SPI_MODE0);
   SPISettings(20000000, MSBFIRST, SPI_MODE1);
   //SPI.begin();
   //make chip selec HIGH
   pinMode(CS,OUTPUT);
   digitalWrite(CS,HIGH);
      
// uncomment for generation of Frequency calibration clock 
//   pinMode(CLK,OUTPUT);
//   TCCR2A = 0x23;
//   TCCR2B = 0x09;
//   OCR2A = 1; //OCR2A = (16,000,000/(frequency))-1  
//   OCR2B = 0; //OCR2B = (16,000,000/(2*frequency))-1  "2" for 50% duty cycle 
//   TCNT2 = 0;
   
   //setup CONFIG1 register
   writeRegister(0x00, 0x80);
   //setup CONFIG2 register
   writeRegister(0x01, 0x41); // sets caibration periods and number of stops
   //setup Interrupt status register
   //writeRegister(0x02, 0x00);
   //setup Interrupt MASK register
   writeRegister(0x03, 0x07);
   //setup coarse counter overflow_H register
   writeRegister(0x04, 0xFF);
   //setup coarse counter overflow_L register
   writeRegister(0x05, 0xFF);
   //setup clock counter overflow_H register
   writeRegister(0x06, 0xFF);
   //setup clock counter overflow_L register
   writeRegister(0x07, 0xFF);
   //setup clock counter stop mask_H register
   writeRegister(0x08, 0x00);
   //setup clock counter stop mask_L register
   writeRegister(0x09, 0x00);
 }
 
 void loop()
 {
   unsigned int TIME1 = 0;
   unsigned int TIME2 = 0;
   unsigned int TIME3 = 0;
   unsigned int TIME4 = 0;
   unsigned int TIME5 = 0;
   unsigned int TIME6 = 0;
   
   unsigned int CLOCK1 = 0;
   unsigned int CLOCK2 = 0;
   unsigned int CLOCK3 = 0;
   unsigned int CLOCK4 = 0;
   unsigned int CLOCK5 = 0;
   unsigned int CLOCK6 = 0;
   
   unsigned int CALIB1 = 0;
   unsigned int CALIB2 = 0;
   int INT_STATUS = 0;
   
   //int CONFIG1 = 0;
   //int CONFIG2 = 0;
   int CLK_OVF = 0;
   int CNT_OVF = 0;
   int MEAS_COM = 0;

   double TOF1 = 0;
   double TOF2 = 0;
   double TOF3 = 0;
   double TOF4 = 0;
   double TOF5 = 0;
   
//// Uncomment the declaration below if writing to EEPROM memory
//   const byte* p1 = (const byte*)(const void*)&TOF1;
//   const byte* p2 = (const byte*)(const void*)&TOF2;
//   const byte* p3 = (const byte*)(const void*)&TOF3;
//   const byte* p4 = (const byte*)(const void*)&TOF4;
//   const byte* p5 = (const byte*)(const void*)&TOF5;

   double CAL_COUNT = 0;
   double NORM_LSB = 0;
   
   //set start measurment bit of CONFIG1 
   writeRegister(0x00, 0x83);
   //select number of stop signals by CONFIG2
   writeRegister(0x01, 0x44);
  
  //wait for the TRIG signal of TDC to go high 
  t = digitalRead(TRIG); 
  while(t != HIGH)
  {
    t = digitalRead(TRIG);
  }
 
// If trig==1 (IC ready) send start and stop signals   
   if(digitalRead(TRIG)==1)
   { 
   //delay(3000);
   digitalWrite(STR,HIGH);
   delayMicroseconds(1);
   digitalWrite(STR,LOW);
   delay(2);
   
   digitalWrite(STR,HIGH);
   delayMicroseconds(1);
   digitalWrite(STR,LOW);
   delay(1);
   
   digitalWrite(STR,HIGH);
   delayMicroseconds(1);
   digitalWrite(STR,LOW);  
   delay(1);
   
   digitalWrite(STR,HIGH);
   delayMicroseconds(1);
   digitalWrite(STR,LOW);
   delay(2);
   
   digitalWrite(STR,HIGH);
   delayMicroseconds(1);
   digitalWrite(STR,LOW);  
   delay(1);
   
   digitalWrite(STR,HIGH);
   delayMicroseconds(1);
   digitalWrite(STR,LOW);
   
   Serial.print("Triggered");
   Serial.print("\n");
   }
   
// INT = LOW indicates measurement complete 
  p = digitalRead(INT); 
  while(p != LOW)
  {
    p = digitalRead(INT);
  }
   
  
   if (digitalRead(INT)== LOW)
  {
    TIME1 = readRegister(0x10, 3);
    CLOCK1 = readRegister(0x11, 3);
    TIME2 = readRegister(0x12, 3);
    CLOCK2 = readRegister(0x13, 3);
    TIME3 = readRegister(0x14, 3); 
    CLOCK3 = readRegister(0x15, 3); 
    TIME4 = readRegister(0x16, 3);
    CLOCK4 = readRegister(0x17, 3);
    TIME5 = readRegister(0x18, 3);
    CLOCK5 = readRegister(0x19, 3); 
    TIME6 = readRegister(0x1A, 3);
    CALIB1 = readRegister(0x1B, 3);
    CALIB2 = readRegister(0x1C, 3);
    
    //delay(500);
   
    INT_STATUS = readRegister(0x02, 1);
//  Bits of interrupt status register    
    CLK_OVF = bitRead(INT_STATUS,2);
    CNT_OVF = bitRead(INT_STATUS,1);
    MEAS_COM = bitRead(INT_STATUS,0);
//    Serial.print("MEAS_COM =");
//    Serial.println(MEAS_COM);
//    Serial.print("CNT_OVF =");
//    Serial.println(CNT_OVF);
//    Serial.print("CLK_OVF =");
//    Serial.println(CLK_OVF);
  }
     
   delay(100);
// Condition to check if measurement complete (always check interrupt pin first) and no overflows
   if (MEAS_COM == 1 & CLK_OVF == 0 & CNT_OVF == 0)
   {
       CAL_COUNT = (CALIB2 - CALIB1) / (CALIB_PERIODS - 1);   
       NORM_LSB = (CLK_PERIOD) / (CAL_COUNT);
       
// For measurement greater than 500ns. if less comment and use below
       TOF1 = (NORM_LSB * (TIME1 - TIME2))+ (CLOCK1 * CLK_PERIOD); 
       TOF2 = (NORM_LSB * (TIME1 - TIME3))+ (CLOCK2 * CLK_PERIOD);  
       TOF3 = (NORM_LSB * (TIME1 - TIME4))+ (CLOCK3 * CLK_PERIOD);  
       TOF4 = (NORM_LSB * (TIME1 - TIME5))+ (CLOCK4 * CLK_PERIOD);  
       TOF5 = (NORM_LSB * (TIME1 - TIME6))+ (CLOCK5 * CLK_PERIOD); 
      
// Uncomment if measuremnt less than 500ns, else use above
//       TOF1 = TIME1 * NORM_LSB;
//       TOF2 = TIME2 * NORM_LSB;
//       TOF3 = TIME3 * NORM_LSB;
//       TOF4 = TIME4 * NORM_LSB;
//       TOF5 = TIME5 * NORM_LSB;
      
       Serial.print(double2s(TOF1,8));
       Serial.print("\n");
       
       Serial.print(double2s(TOF2,8));
       Serial.print("\n");
       
       Serial.print(double2s(TOF3,8));
       Serial.print("\n");
       
       Serial.print(double2s(TOF4,8));
       Serial.print("\n");
       
       Serial.print(double2s(TOF5,8));
       Serial.print("\n");
       
// Uncomment to write to EEPROM. Limited memory hence addr<=440 condition                
//            if (addr<=440)
//            {              
//              for(int i=0;i<sizeof(double2s(TOF1,4))-1;i++){
//              EEPROM.write(addr++, *p1++);}
//              EEPROM.write(addr++, '\n');
//                            
//              for(int i=0;i<sizeof(double2s(TOF2,4))-1;i++){
//              EEPROM.write(addr++, *p2++);}
//              EEPROM.write(addr++, '\n');
//              
//              for(int i=0;i<sizeof(double2s(TOF3,4))-1;i++){
//              EEPROM.write(addr++, *p3++);}
//              EEPROM.write(addr++, '\n');
//              
//              for(int i=0;i<sizeof(double2s(TOF4,4))-1;i++){
//              EEPROM.write(addr++, *p4++);}
//              EEPROM.write(addr++, '\n');
//              
//              for(int i=0;i<sizeof(double2s(TOF5,4))-1;i++){
//              EEPROM.write(addr++, *p5++);}
//              EEPROM.write(addr++, '\n');
//            }
             
////Uncomment to debug             
//     CONFIG1 = readRegister(0x00, 1);
//     CONFIG2 = readRegister(0x01, 1);
//     Serial.print("CONFIG1 = ");
//     Serial.println(CONFIG1);
//     Serial.print("CONFIG2 = ");
//     Serial.println(CONFIG2);
//     Serial.print("TIME1 = ");
//     Serial.println(TIME1);
//     Serial.print("TIME2 = ");
//     Serial.println(TIME2  );
//     Serial.print("TIME3 = ");
//     Serial.println(TIME3);
//     Serial.print("TIME4 = ");
//     Serial.println(TIME4);
//     Serial.print("TIME5 = ");
//     Serial.println(TIME5  );
//     Serial.print("TIME6= ");
//     Serial.println(TIME6);
//     Serial.print("CLOCK1 = ");
//     Serial.println(CLOCK1);
//     Serial.print("CLOCK2 = ");
//     Serial.println(CLOCK2  );
//     Serial.print("CLOCK3 = ");
//     Serial.println(CLOCK3);
//     Serial.print("CLOCK4 = ");
//     Serial.println(CLOCK4  );
//     Serial.print("CLOCK5 = ");
//     Serial.println(CLOCK5  );
//     Serial.print("CALIB1 = ");
//     Serial.println(CALIB1);
//     Serial.print("CALIB2 = ");
//     Serial.println(CALIB2  );
//     Serial.print("CAL_COUNT = ");
//     Serial.println(CAL_COUNT);
//     Serial.println();
   }

//write to interrupt status register(02h) to clear all the status
   writeRegister(0x02, 0x1F);
   delay(500);
}
// Serial.print("i = ");
// Serial.println(i);
// Serial.println();
//}
 
unsigned int readRegister(byte address, int datalength)
 {
   byte data = 0;
   int fulldata = 0;
   address = address & READ;
   
   digitalWrite(CS,LOW);
   SPI.transfer(address);
   fulldata = SPI.transfer(0x00);
   datalength--;
   
   while (datalength>0)
   {
     fulldata = fulldata << 8;
     data = SPI.transfer(0x00);
     fulldata = fulldata | data;
     //Serial.println(fulldata);
     datalength--;
   }
   
   digitalWrite(CS,HIGH);
   return(fulldata);
  }

 void writeRegister(byte address, byte value)
 {
   address = address | WRITE;
   digitalWrite(CS,LOW);
 
   SPI.transfer(address);
   SPI.transfer(value);
 
   digitalWrite(CS,HIGH); 
 }  
 
char * double2s(double f, unsigned int digits)
{
 int index = 0;
 static char s[16];                    // buffer to build string representation

 // max digits
 if (digits > 6) digits = 6;
 long multiplier = pow(10, digits);     // fix int => long

 int exponent = int(log10(f));
 float g = f / pow(10, exponent);
 if ((g < 1.0) && (g != 0.0))      
 {
   g *= 10;
   exponent--;
 }

 long whole = long(g);                     // single digit
 long part = long((g-whole)*multiplier);   // # digits
 char format[16];
 sprintf(format, "%%ld.%%0%dld E%%+d", digits);
 sprintf(&s[index], format, whole, part, exponent);
 
 return s;
}

