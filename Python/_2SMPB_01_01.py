# Distributed with a free-will license.
# Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
# 2SMPB-01-01
# This code is designed to work with the 2SMPB-01-01_I2CS I2C Mini Module available from ControlEverything.com.
# https://www.controleverything.com/products

import smbus
import time

# Get I2C bus
bus = smbus.SMBus(1)

# 2SMPB-01-01 address, 0x70(112)
# Select sensor request register, 0x03(03)
#		0x14(20)	Wake up the system, normal mode
bus.write_byte_data(0x70, 0x03, 0x14)
# 2SMPB-01-01 address, 0x70(112)
# Select OTP read register, 0x08(08)
#		0x25(37)	Enable OTP read
bus.write_byte_data(0x70, 0x08, 0x25)

time.sleep(0.5)

# Calliberation Cofficients
# Read data back from 0x26(38), 2 bytes
# Bt MSB, Bt LSB
data = bus.read_i2c_block_data(0x70, 0x26, 2)

# Convert the data
Bt = data[0] * 256 + data[1]

# Read data back from 0x28(40), 2 bytes
# Ct MSB, Ct LSB
data = bus.read_i2c_block_data(0x70, 0x28, 2)

# Convert the data
Ct = data[0] * 256 + data[1]

# Read data back from 0x2A(42), 2 bytes
# Bt2 MSB, Bt2 LSB
data = bus.read_i2c_block_data(0x70, 0x2A, 2)

# Convert the data
Bt2 = data[0] * 256 + data[1]

# Read data back fropm 0x2C(44), 2 bytes
# Ct2 MSB, Ct2 LSB
data = bus.read_i2c_block_data(0x70, 0x2C, 2)

# Convert the data
Ct2 = data[0] * 256 + data[1]

# Read data back from 0x2E(46), 2 bytes
# Ba MSB, Ba LSB
data = bus.read_i2c_block_data(0x70, 0x2E, 2)

# Convert the data
Ba = data[0] * 256 + data[1]

# Read data back from 0x22(34), 2 bytes
# Bp MSB, Bp LSB
data = bus.read_i2c_block_data(0x70, 0x22, 2)

# Convert the data
Bp = data[0] * 256 + data[1]

# Read data back from 0x24(36), 2 bytes
# Cp MSB, Cp LSB
data1 = bus.read_i2c_block_data(0x70, 0x24, 2)

# Read data back from 0x32(50), 2 bytes
# Ca MSB, Ca LSB
data2 = bus.read_i2c_block_data(0x70, 0x32, 2)

# Read data back from 0x34(52), 2 bytes
# Cp SLSB, Ca SLSB
data3 = bus.read_i2c_block_data(0x70, 0x34, 2)

# Convert the data
Cp = (data3[0] * 65536) + (data1[0] * 256) + data1[1]
Ca = (data3[1] * 65536) + (data2[0] * 256) + data2[1]

time.sleep(0.5)

# 2SMPB-01-01 address, 0x70(112)
# Select OTP read register, 0x08(08)
#		0x65(101)	Disable OTP read
bus.write_byte_data(0x70, 0x08, 0x65)
# 2SMPB-01-01 address, 0x70(112)
# Select set-up register, 0x0A(10)
#		0x02(02)	Start ADC operation
bus.write_byte_data(0x70, 0x0A, 0x02)
# 2SMPB-01-01 address, 0x70(112)
# Select sensor request register, 0x03(03)
#		0x17(23)	Wake up the system, high accuracy mode, Enable measurement for PTAT
bus.write_byte_data(0x70, 0x03, 0x17)

time.sleep(0.5)

# 2SMPB-01-01 address, 0x70(112)
# Read data back from 0x00(00), 3 bytes
# Dt MSB1, Dt MSB, Dt LSB
data = bus.read_i2c_block_data(0x70, 0x00, 3)

# Convert the data
Dt = (data[0] * 65536) + (data[1] * 256) + data[2]

# First order compensation
temp = (((Dt - Ca) * Ba )/ 524288.0) / 256.0

# 2SMPB-01-01 address, 0x70(112)
# Select sensor request register, 0x03(03)
#		0x16(22)	Wake up the system, high accuracy mode, Enable measurement for pressure
bus.write_byte_data(0x70, 0x03, 0x16)

time.sleep(0.5)

# 2SMPB-01-01 address, 0x70(112)
# Read data back from 0x00(00), 3 bytes
# Dp MSB1, Dp MSB, Dp LSB
data = bus.read_i2c_block_data(0x70, 0x00, 3)

# Convert the data
Dp = (data[0] * 65536) + (data[1] * 256) + data[2]

# First order compensation
pres = ((Dp - Cp) * Bp) / 524288.0

# Second order compensation
# Relative pressure
if temp >= 25 :
	pres = (pres + ((pres + 90000.0) * ((Ct / 262144.0) + (Bt / 2147483648.0 * temp)))) / 1000.0
else :
	pres = (pres + ((pres + 90000.0) * ((Ct2 / 262144.0) + (Bt2 / 2147483648.0 * temp)))) / 1000.0

# Absolute Pressure
pressure  = 90.0 + pres

# Output data to screen
print "Pressure : %.2f " %pressure
