
#include <Wire.h>

#define PCF8583_ADDRESS 0x50  // Default I2C address

bool hasRun = false;
  int nDevices = 0;
//
byte testRAMAddr = 0x08;
byte testRAMValue = 0xAB;

// BCD to decimal conversion
int bcdToDec(byte val) {
  return ((val >> 4) * 10 + (val & 0x0F));
}

void setup() {
  Serial.begin(9600);
  Serial.print("\n\n----------------------------------------------\n");
  Wire.begin(9600);

  byte error, address;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("> Device found at 0x");
      Serial.println(address, HEX);
      nDevices++;
    } else if (error == 4) {
      Serial.print("> Unknown error at 0x");
      Serial.println(address, HEX);
    };
  };

  if (!nDevices){
    Serial.println("> No I2C devices found");
  } else {
    // Set time to 12:34:56
    Wire.beginTransmission(PCF8583_ADDRESS);
    Wire.write(0x02); // Start at hundredths
    Wire.write(0x00); // hundredths
    Wire.write(0x56); // seconds (BCD 56)
    Wire.write(0x34); // minutes (BCD 34)
    Wire.write(0x12); // hours   (BCD 12)

    //Wire.write(0x12); // Hours
    Wire.endTransmission();

    // === Write test value to RAM ===
    Wire.beginTransmission(PCF8583_ADDRESS);
    Wire.write(testRAMAddr);        // RAM address
    Wire.write(testRAMValue);       // Test value
    Wire.endTransmission();
    delay(100);  // Wait a bit before reading

    // === Read back RAM value ===
    Wire.beginTransmission(PCF8583_ADDRESS);
    Wire.write(testRAMAddr);
    Wire.endTransmission();
    Wire.requestFrom(PCF8583_ADDRESS, 1);
    byte readValue = Wire.read();

    // === Compare and report ===
    Serial.print("> RAM Test - Written: 0x");
    Serial.print(testRAMValue, HEX);
    Serial.print(" | Read: 0x");
    Serial.println(readValue, HEX);

    if (readValue == testRAMValue) {
      Serial.println("> RAM test PASSED!");
    } else {
      Serial.println("> RAM test FAILED!");
    }
    delay(500);
  };
}

void loop() {

  if (!hasRun && nDevices) {
    hasRun = true;

    // Optional: print time every 2s
    Wire.beginTransmission(PCF8583_ADDRESS);
    Wire.write(0x03);
    Wire.endTransmission();

    Wire.requestFrom(PCF8583_ADDRESS, 3);
    byte secBCD = Wire.read();  // Seconds
    byte minBCD = Wire.read();  // Minutes
    byte hourBCD = Wire.read() & 0x3F; // Mask upper bits; // Hours

    int sec = bcdToDec(secBCD);
    int min = bcdToDec(minBCD);
    int hr  = bcdToDec(hourBCD);

    Serial.print("Time: ");
    if (hr < 10) Serial.print("0");
    Serial.print(hr); Serial.print(":");

    if (min < 10) Serial.print("0");
    Serial.print(min); Serial.print(":");

    if (sec < 10) Serial.print("0");
    Serial.println(sec);

    delay(2000);
    //
    //
  } else {
    Serial.print("\n> Reset to run again!");
    while(true);
  }

}
