// http://playground.arduino.cc/Learning/OneWire
#include <OneWire.h>

// DS18S20 Temperature chip i/o
OneWire ds(9);  // on pin 10
const int motor = 12;
const int indicador = 13;
int ligado = 0;
long counter = 0;
int first = 1;
int lastSignBit;
int lastTemperature;
int myTemps[] = {0,0,0,0,0,0,0,0,0,0};

void setup(void) { // initialize inputs/outputs & start serial port
  Serial.begin(9600);
  pinMode (motor, OUTPUT);
  pinMode (indicador, OUTPUT);
}

// First off, you need to define some variables, (put right under loop() above)
int HighByte, LowByte, TReading, SignBit, Tc_100, Temperatura, Fract;
void loop(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  ds.reset_search();
  if ( !ds.search(addr)) {
      if (ligado == 0 && ( lastSignBit != -32768 && lastTemperature >= 2 ) ){
        Serial.print("No more addresses.\n");
        Serial.print("motor ligado.\n");
        Serial.print("ultima temp: ");
        Serial.println(lastTemperature);
        digitalWrite (motor, HIGH);
        ligado = 1;
      }
      ds.reset_search();
      return;
  }
  ligado = 0;
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      if (ligado == 0){
        Serial.print("motor ligado.\n");
        digitalWrite (motor, HIGH);
        ligado = 1;
      }
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4; // multiply by (100 * 0.0625) or 6.25

  Temperatura = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
//  AQUI TENHO A ESCRITA DA TEMPERATURA!!!!!
  if (SignBit) // If its negative
  {
     Serial.print("-");
  }
  Serial.print(Temperatura);
  Serial.print(".");
  if (Fract < 10)
  {
     Serial.print("0");
  }
  Serial.print(Fract);
  Serial.print("\n");  
//  OPERAÇÃO COM MOTOR

  if (SignBit != -32768 && Temperatura >= 2){ // compara se a temperatura é + ou - e compara com temperatura máxima precetada
   digitalWrite (motor, HIGH);
  }
  else if (SignBit == -32768 && Temperatura >= 15) { // compara se a temperatura é + ou - e compara com temperatura mínima precetada
   digitalWrite (motor, LOW);
  }
  lastSignBit = SignBit;
  lastTemperature = Temperatura;
}
