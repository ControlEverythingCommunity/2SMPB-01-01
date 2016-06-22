// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// 2SMPB-01-01
// This code is designed to work with the 2SMPB-01-01_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/products

import com.pi4j.io.i2c.I2CBus;
import com.pi4j.io.i2c.I2CDevice;
import com.pi4j.io.i2c.I2CFactory;
import java.io.IOException;

public class _2SMPB_01_01
{
	public static void main(String args[]) throws Exception
	{
		// Create I2C bus
		I2CBus Bus = I2CFactory.getInstance(I2CBus.BUS_1);
		// Get I2C device, 2SMPB_01_01 I2C address is 0x70(112)
		I2CDevice device = Bus.getDevice(0x70);
		
		// Select sensor request register
		// wake up the system, normal mode
		device.write(0x03, (byte)0x14);
		Thread.sleep(50);
		// Select OTP read register
		// Enable OTP read
		device.write(0x08, (byte)0x25);
		Thread.sleep(300);
		
		// Calliberation Cofficients
		// Read 2 bytes of data from address 0x22(34)
		// Bp msb, Bp lsb
		byte[] data = new byte[3];
		device.read(0x22, data, 0, 2);
		long Bp = (((data[0] & 0xFF) * 256) + (data[1] & 0xFF));
		
		// Read 2 bytes of data from address 0x24(36)
		// Cp msb, Cp lsb
		byte[] data1 = new byte[2];
		device.read(0x24, data1, 0, 2);
		
		// Read 2 bytes of data from address 0x26(38)
		// Bt msb, Bt lsb
		device.read(0x26, data, 0, 2);
		long Bt = (((data[0] & 0xFF) * 256) + (data[1] & 0xFF));
		
		// Read 2 bytes of data from address 0x28(40)
		// Ct msb, Ct lsb
		device.read(0x28, data, 0, 2);
		long Ct = (((data[0] & 0xFF) * 256) + (data[1] & 0xFF));
		
		// Read 2 bytes of data from address 0x2A(42)
		// Bt2 msb, Bt2 lsb
		device.read(0x2A, data, 0, 2);
		long Bt2 = (((data[0] & 0xFF) * 256) + (data[1] & 0xFF));
		
		// Read 2 bytes of data from address 0x2C(44)
		// Ct2 msb, Ct2 lsb
		device.read(0x2C, data, 0, 2);
		long Ct2 = (((data[0] & 0xFF) * 256) + (data[1] & 0xFF));
		
		// Read 2 bytes of data from address 0x2E(46)
		// Ba msb, Ba lsb
		device.read(0x2E, data, 0, 2);
		long Ba = (((data[0] & 0xFF) * 256) + (data[1] & 0xFF));
		
		// Read 2 bytes of data from address 0x32(50)
		// Ca msb, Ca lsb
		byte[] data2 = new byte[2];
		device.read(0x32, data2, 0, 2);
		
		// Read 2 bytes of data from address 0x34(52)
		// Cp msb1, Ca msb1
		device.read(0x34, data, 0, 2);
		
		long Ca = ((data[1] & 0xFF) * 65536 + (data2[0] & 0xFF) * 256 + (data2[1] & 0xFF));
		long Cp = ((data[0] & 0xFF) * 65536 + (data1[0] & 0xFF) * 256 + (data1[1] & 0xFF));
		
		// Select OTP read register
		// Disable OTP read
		device.write(0x08, (byte)0x65);
		
		// Select set-up register
		// Start ADC operation
		device.write(0x0A, (byte)0x02);
		Thread.sleep(100);
		
		// Select sensor request register
		// Wake up the system, high accuracy mode, Enable measurement for PTAT
		device.write(0x03, (byte)0x17);
		Thread.sleep(500);
		
		// Read 3 bytes of data from address 0x00(00)
		// Dt msb1, Dt msb, Dt lsb
		device.read(0x00, data, 0, 3);
		
		// Convert the data
		long Dt = ((data[0] & 0xFF) * 65536 + (data[1] & 0xFF) * 256 + (data[2] & 0xFF));
		
		// First order compensation
		double temp = (((Dt - Ca) * Ba )/ 524288.0) / 256.0;
		
		// Select sensor request register
		// Wake up the system, high accuracy mode, Enable measurement for pressure
		device.write(0x03, (byte)0x16);
		Thread.sleep(500);
		
		// Read 3 bytes of data from address 0x00(00)
		// Dp msb1, Dp msb, Dp lsb
		device.read(0x00, data, 0, 3);
		
		// Convert the data
		long Dp = ((data[0] & 0xFF) * 65536 + (data[1] & 0xFF) * 256 + (data[2] & 0xFF));
		
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
		
		// Output data to the screen
		System.out.printf("Pressure : %.2f kPa %n", pressure);
	}
}