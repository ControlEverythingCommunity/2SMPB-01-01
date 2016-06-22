// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// 2SMPB-01-01
// This code is designed to work with the 2SMPB-01-01_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/products

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

void main() 
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-1";
	if ((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, 2SMPB-01-01 I2C address is 0x70(112)
	ioctl(file, I2C_SLAVE, 0x70);

	// Select sensor request register(0x03)
	// wake up the system, normal mode(0x14)
	char config[2] = {0};
	config[0] = 0x03;
	config[1] = 0x14;
	write(file, config, 2);
	sleep(0.5);

	// Select OTP read register(0x08)
	// Enable OTP read(0x25)
	config[0] = 0x08;
	config[1] = 0x25;
	write(file, config, 2);
	sleep(1);

	// Calliberation Cofficients
	// Read 2 bytes of data from register(0x22)
	// Bp msb, Bp lsb
	char reg[1] = {0x22};
	write(file, reg, 1);
	char data[2] = {0};
	if(read(file, data, 2) != 2) 
	{
		printf("Error : Input/output Error \n");
	}
	else
	{
	int Bp = data[0] * 256 + data[1];

	// Read 2 bytes of data from register(0x24)
	// Cp msb, Cp lsb
	reg[0] = 0x24;
	write(file, reg, 1);
	char data1[2] = {0};
	read(file, data1, 2);

	// Read 2 bytes of data from address(0x26)
	// Bt msb, Bt lsb
	reg[0] = 0x26;
	write(file, reg, 1);
	read(file, data, 2);;
	int Bt = data[0] * 256 + data[1];

	// Read 2 bytes of data from address(0x28)
	// Ct msb, Ct lsb
	reg[0] = 0x28;
	write(file, reg, 1);
	read(file, data, 2);;
	int Ct = data[0] * 256 + data[1];

	// Read 2 bytes of data from address(0x2A)
	// Bt2 msb, Bt2 lsb
	reg[0] = 0x2A;
	write(file, reg, 1);
	read(file, data, 2);
	int Bt2 = data[0] * 256 + data[1];

	// Read 2 bytes of data from address(0x2C)
	// Ct2 msb, Ct2 lsb
	reg[0] = 0x2C;
	write(file, reg, 1);
	read(file, data, 2);
	int Ct2 = data[0] * 256 + data[1];

	// Read 2 bytes of data from address(0x2E)
	// Ba msb, Ba lsb
	reg[0] = 0x2E;
	write(file, reg, 1);
	read(file, data, 2);
	int Ba = data[0] * 256 + data[1];

	// Read 2 bytes of data from address(0x32)
	// Ca msb, Ca lsb
	reg[0] = 0x32;
	write(file, reg, 1);
	char data2[2] = {0};
	read(file, data2, 2);

	// Read 2 bytes of data from address(0x34)
	// Cp msb1, Ca msb1
	reg[0] = 0x34;
	write(file, reg, 1);
	read(file, data, 2);

	int Ca = data[1] * 65536 + data2[0] * 256 + data2[1];
	int Cp = data[0] * 65536 + data1[0] * 256 + data1[1];

	// Select OTP read register(0x08)
	// Disable OTP read(0x65)
	config[0] = 0x08;
	config[1] = 0x65;
	write(file, config, 2);
	// Select set-up register(0x0A)
	// Start ADC operation(0x02)
	config[0] = 0x0A;
	config[1] = 0x02;
	write(file, config, 2);
	sleep(0.5);
	// Select sensor request register(0x03)
	// Wake up the system, high accuracy mode, Enable measurement for PTAT(0x17)
	config[0] = 0x03;
	config[1] = 0x17;
	write(file, config, 2);
	sleep(1);

	// Read 3 bytes of data from register(0x00)
	// Dt msb1, Dt msb, Dt lsb
	reg[0] = 0x00;
	write(file, reg, 1);
	read(file, data, 3);

	// Convert the data
	double Dt = data[0] * 65536 + data[1] * 256 + data[2];

	// First order compensation
	double temp = (((Dt - Ca) * Ba )/ 524288.0) / 256.0;

	// Select sensor request register(0x03)
	// Wake up the system, high accuracy mode, Enable measurement for pressure(0x16)
	config[0] = 0x03;
	config[1] = 0x16;
	write(file, config, 2);
	sleep(1);

	// Read 3 bytes of data from register(0x00)
	// Dp msb1, Dp msb, Dp lsb
	reg[0] = 0x00;
	write(file, reg, 1);
	read(file, data, 3);

	// Convert the data
	double Dp = data[0] * 65536 + data[1] * 256 + data[2];

	// First order compensation
	double pres = ((Dp - Cp) * Bp) / 524288.0;

	// Second order compensation
	// Relative pressure
	if(temp >= 25.0)
	{
		pres = (pres + ((pres + 90000.0) * (((double)Ct / 262144.0) + ((double)Bt / 2147483648.0 * temp)))) / 1000.0;
	}
	else
	{
		pres = pres + ((pres + 90000.0) * (((double)Ct2 / 262144.0) + ((double)Bt2 / 2147483648.0 * temp))) / 1000.0;
	}

	// Absolute pressure
	double pressure = 90.0 + pres;

	// Output data to screen
	printf("Pressure : %.2f kPa \n", pressure);
	}
}
