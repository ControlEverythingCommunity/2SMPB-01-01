// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// 2SMPB-01-01
// This code is designed to work with the 2SMPB-01-01_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/products

#include <application.h>
#include <spark_wiring_i2c.h>

// 2SMPB-01-01 I2C address is 0x70(112)
#define Addr 0x70

double pressure = 0.0;
void setup()
{
  // Set variable
  Particle.variable("i2cdevice", "2SMPB-01-01");
  Particle.variable("pressure", pressure);

  // Initialise I2C communication as MASTER
  Wire.begin();
  // Initialise Serial communication, set baud rate = 9600
  Serial.begin(9600);
  delay(300);
}

void loop()
{
  unsigned int data[14];
  unsigned int data1[4];

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select sensor request register
  Wire.write(0x03);
  // wake up the system, normal mode
  Wire.write(0x14);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select OTP read register
  Wire.write(0x08);
  // Enable OTP read
  Wire.write(0x25);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);

  for (int i = 0; i < 14; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write((34 + i));
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 1 byte of data
    Wire.requestFrom(Addr, 1);

    // Read 1 byte of data
    if (Wire.available() == 1)
    {
      data[i] = Wire.read();
    }
  }

  // Calliberation Cofficients
  int Bp = (data[0] * 256) + data[1];
  int Bt = (data[4] * 256) + data[5];
  int Ct = (data[6] * 256) + data[7];
  int Bt2 = (data[8] * 256) + data[9];
  int Ct2 = (data[10] * 256) + data[11];
  int Ba = (data[12] * 256) + data[13];

  for (int j = 0; j < 4; j++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write((50 + j));
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 1 byte of data
    Wire.requestFrom(Addr, 1);

    // Read 1 byte of data
    if (Wire.available() == 1)
    {
      data1[j] = Wire.read();
    }
  }
  // Calliberation Cofficients
  int Ca = (data1[3] * 65536) + (data1[0] * 256) + data1[1];
  int Cp = (data1[2] * 65536) + (data[2] * 256) + data[3];

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select OTP read register
  Wire.write(0x08);
  // Disable OTP read
  Wire.write(0x65);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select set-up register
  Wire.write(0x0A);
  // Start ADC operation
  Wire.write(0x02);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select sensor request register
  Wire.write(0x03);
  // Wake up the system, high accuracy mode, Enable measurement for PTAT
  Wire.write(0x17);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Request 3 bytes of data
  Wire.requestFrom(Addr, 3);

  // Read 3 bytes of data
  // Dt msb1, Dt msb, Dt lsb
  if (Wire.available() == 3)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
  }

  // Convert the data
  long Dt = (data[0] * 65536) + (data[1] * 256) + data[2];

  // First order compensation
  double temp = (((Dt - Ca) * Ba) / 524288.0) / 256.0;

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select sensor request register
  Wire.write(0x03);
  // Wake up the system, high accuracy mode, Enable measurement for pressure
  Wire.write(0x16);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Request 3 bytes of data
  Wire.requestFrom(Addr, 3);

  // Read 3 bytes of data
  // Dp msb1, Dp msb, Dp lsb
  if (Wire.available() == 3)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
  }

  // Convert the data
  long Dp = (data[0] * 65536) + (data[1] * 256) + data[2];

  // First order compensation
  double pres = ((Dp - Cp) * Bp) / 524288.0;

  // Second order compensation
  // Relative pressure
  if (temp >= 25.0)
  {
    pres = (pres + ((pres + 90000.0) * (((double)Ct / 262144.0) + ((double)Bt / 2147483648.0 * temp)))) / 1000.0;
  }
  else
  {
    pres = (pres + ((pres + 90000.0) * (((double)Ct2 / 262144.0) + ((double)Bt2 / 2147483648.0 * temp)))) / 1000.0;
  }

  // Absolute pressure
  pressure = 90.0 + pres;

  // Output data to dashboard
  Particle.publish("Pressure : ", String(pressure));
  delay(1000);
}
