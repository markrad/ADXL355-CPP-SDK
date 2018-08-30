#include <wiringPiI2C.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <cstring>

#include "ADXL355.h"

using namespace std;

ADXL355::ADXL355(int deviceId) : _deviceId(deviceId)
{
    _fd = wiringPiI2CSetup(_deviceId);
}

ADXL355::~ADXL355()
{
}

uint8_t ADXL355::GetAnalogDevicesID()
{
    CheckConnection();

    int work = Read8(DEVID_AD);

    return work;
}

uint8_t ADXL355::GetAnalogDevicesMEMSID()
{
    CheckConnection();

    int work = Read8(DEVID_MST);

    return work;
}

uint8_t ADXL355::GetDeviceId()
{
    CheckConnection();

    int work = Read8(PARTID);

    return work;
}

uint8_t ADXL355::GetRevision()
{
    CheckConnection();

    int work = Read8(REVID);

    return work;
}

ADXL355::HPF_CORNER ADXL355::GetHpfCorner()
{
    CheckConnection();

        int work = Read8(FILTER);

    return (HPF_CORNER)((work & HPF_CORNER::HPF_CORNER_MASK) >> 4);
}

ADXL355::ODR_LPF ADXL355::GetOdrLpf()
{
    CheckConnection();

    int work = Read8(FILTER);

    return (ODR_LPF)(work & ODR_LPF::ODR_LPF_MASK);
}

void ADXL355::SetHpfCorner(HPF_CORNER value)
{
    CheckConnection();
    ErrorIfRunning();

    int work = Read8(FILTER);

    work = (work & ~(HPF_CORNER::HPF_CORNER_MASK << 4)) | ((int)value) << 4;
	
    if (Write8(FILTER, work) < 0)
		WriteFailedException();
}

void ADXL355::SetOdrLpf(ODR_LPF value)
{
    CheckConnection();
    ErrorIfRunning();
    
    int work = Read8(FILTER);

    work = (work & ~(ODR_LPF::ODR_LPF_MASK)) | ((int)value);
	
    if (Write8(FILTER, work) < 0)
		WriteFailedException();
}

bool ADXL355::IsI2CSpeedFast()
{
    CheckConnection();
    
	int work = Read8(I2CSPEED_INTPOLARITY_RANGE);
	
	return (bool)(work & I2C_SPEED_VALUES::I2C_SPEED_FAST);
}

void ADXL355::SetI2CSpeed(bool value)
{
    CheckConnection();
    ErrorIfRunning();
    
	int work = Read8(I2CSPEED_INTPOLARITY_RANGE);
	
	if (value != (bool)(work & I2C_SPEED_VALUES::I2C_SPEED_FAST))
	{
		work = (value)? (work | I2C_SPEED_VALUES::I2C_SPEED_FAST) : (work & ~I2C_SPEED_VALUES::I2C_SPEED_FAST);
		
		if (Write8(I2CSPEED_INTPOLARITY_RANGE, work) < 0)
			WriteFailedException();
	}
}

int ADXL355::GetFifoCount()
{
    CheckConnection();

    int work = Read8(FIFO_ENTRIES);

    return work;
}

ADXL355::STATUS_VALUES ADXL355::GetStatus()
{
    CheckConnection();

    int work = Read8(STATUS);

    return (STATUS_VALUES)work;
}

bool ADXL355::IsFifoFull()
{
    STATUS_VALUES work = GetStatus();

    return (work & STATUS_VALUES::FIFO_FULL)? true : false;
}

bool ADXL355::IsFifoOverrun()
{
    STATUS_VALUES work = GetStatus();

    return (work & STATUS_VALUES::FIFO_OVERRUN)? true : false;
}

bool ADXL355::IsDataReady()
{
    STATUS_VALUES work = GetStatus();

    return (work & STATUS_VALUES::DATA_READY)? true : false;
}

void ADXL355::Start()
{
	CheckConnection();
	
	int work = Read8(POWER_CTL);

	if (work & POWER_CTL_VALUES::POWER_CTL_OFF)
	{
		work = work & (int)POWER_CTL_VALUES::POWER_CTL_ON;
		
		if (Write8(POWER_CTL, work) < 0)
			WriteFailedException();
	}
}

void ADXL355::Stop()
{
	CheckConnection();

	int work = Read8(POWER_CTL);

	if (!(work & POWER_CTL_VALUES::POWER_CTL_OFF))
	{
		work = work | (int)POWER_CTL_VALUES::POWER_CTL_OFF;
		
		if (Write8(POWER_CTL, work) < 0)
			WriteFailedException();
	}
}

void ADXL355::Reset()
{
    CheckConnection();

    if (Write8(RESET, RESET_VALUE) < 0)
        WriteFailedException();
}

bool ADXL355::IsRunning()
{
	CheckConnection();

    bool result = false;

	int work = Read8(POWER_CTL);

	result = (work & POWER_CTL_VALUES::POWER_CTL_OFF)
		? false
		: true;

    return result;
}

bool ADXL355::IsTempSensorOn()
{
	CheckConnection();

    bool result = false;
	
	int work = Read8(POWER_CTL);

	result = ((work & POWER_CTL_VALUES::POWER_CTL_OFF) || (work & POWER_CTL_VALUES::POWER_CTL_TEMP_OFF))
		? false
		: true;

    return result;
}

void ADXL355::StartTempSensor()
{
	CheckConnection();

	int work = Read8(POWER_CTL);

	if (work & POWER_CTL_VALUES::POWER_CTL_TEMP_OFF)
	{
		work = work & (int)POWER_CTL_VALUES::POWER_CTL_TEMP_ON;
		
		if (Write8(POWER_CTL, work) < 0)
			WriteFailedException();
	}
}

void ADXL355::StopTempSensor()
{
	CheckConnection();

	int work = Read8(POWER_CTL);

	if (!(work & POWER_CTL_VALUES::POWER_CTL_TEMP_OFF))
	{
		work = work | (int)POWER_CTL_VALUES::POWER_CTL_TEMP_OFF;
		
		if (Write8(POWER_CTL, work) < 0)
			WriteFailedException();
	}
}

double ADXL355::GetTemperatureC()
{
	CheckConnection();

    int work = Read16(TEMP2);
    //int check 1 = (w)
    int itemp = ((work & 0xff) << 8) | (work >> 8);

    double dtemp = ((double)(1852 - itemp)) / 9.05 + 19.21;

	return dtemp;
}

double ADXL355::GetTemperatureF()
{
    double result = GetTemperatureC();

    return result * 9 / 5 + 32;
}

int ADXL355::GetRawAxes(long *x, long *y, long *z)
{
	CheckConnection();

    uint8_t output[9];
    memset(output, 0, 9);

    int result = ReadBlock(XDATA3, 9, (uint8_t *)output);

    unsigned long workx;
    unsigned long worky;
    unsigned long workz;

    if (result == 0)
    {
        workx = (output[0] << 12) | (output[1] << 4) | (output[2] >> 4);
        worky = (output[3] << 12) | (output[4] << 4) | (output[5] >> 4);
        workz = (output[6] << 12) | (output[7] << 4) | (output[8] >> 4);
    }

    *x = TwosCompliment(workx);
    *y = TwosCompliment(worky);
    *z = TwosCompliment(workz);

    return result;
}

int ADXL355::GetTrim(int32_t *x, int32_t *y, int32_t *z)
{
    CheckConnection();
    
    uint8_t output[6];

    memset(output, 0xff, sizeof(output));

    int result = ReadBlock(OFFSET_X_H, 6, (uint8_t *)output);

    if (result == 0)
    {
        *x = TwosCompliment((output[0] << 8 | output[1]) << 4);
        *y = TwosCompliment((output[2] << 8 | output[3]) << 4);
        *z = TwosCompliment((output[4] << 8 | output[5]) << 4);
    }

    return result;
}

void ADXL355::SetTrim(int32_t x, int32_t y, int32_t z)
{
	CheckConnection();
    ErrorIfRunning();

    int16_t workx = (x >> 4);
    int16_t worky = (y >> 4);
    int16_t workz = (z >> 4);
    uint8_t hix = (workx & 0xff00) >> 8;
    uint8_t lox = workx & 0x00ff;
    uint8_t hiy = (worky & 0xff00) >> 8;
    uint8_t loy = worky & 0x00ff;
    uint8_t hiz = (workz & 0xff00) >> 8;
    uint8_t loz = workz & 0x00ff;

    if (Write8(OFFSET_X_H, hix) < 0)
		WriteFailedException();

    if (Write8(OFFSET_X_L, lox) < 0)
		WriteFailedException();
    
    if (Write8(OFFSET_Y_H, hiy) < 0)
		WriteFailedException();

    if (Write8(OFFSET_Y_L, loy) < 0)
		WriteFailedException();

    if (Write8(OFFSET_Z_H, hiz) < 0)
	 	WriteFailedException();

    if (Write8(OFFSET_Z_L, loz) < 0)
	 	WriteFailedException();
}

int ADXL355::ReadFifoEntries(long *output)
{
	CheckConnection();

    int fifoCount = GetFifoCount();
    uint8_t data[9];
    memset(data, 0, 9);

    unsigned long work[3];

    for (int i = 0; i < fifoCount / 3; i++)
    {
        int result = ReadBlock(FIFO_DATA, 9, (uint8_t *)data);

        if (result == 0)
        {
            for (int j = 0; j < 9; j+= 3)
            {
                work[j / 3] = (data[0 + j] << 12) | (data[1 + j] << 4) | (data[2 + j] >> 4);
                output[i * 3 + j / 3] = TwosCompliment(work[j / 3]); 
            }
        }
        else
        {
            return -1;
        }
    }

    return fifoCount / 3;
}


ADXL355::RANGE_VALUES ADXL355::GetRange()
{
	CheckConnection();

    int range = Read8(I2CSPEED_INTPOLARITY_RANGE);

    return (RANGE_VALUES)(range & RANGE_VALUES::RANGE_MASK);
}

void ADXL355::SetRange(RANGE_VALUES value)
{
    CheckConnection();
    ErrorIfRunning();
    
    int range = Read8(I2CSPEED_INTPOLARITY_RANGE);

    range &= ~(RANGE_VALUES::RANGE_MASK);
    range |= (int)value;

    if (Write8(I2CSPEED_INTPOLARITY_RANGE, range) < 0)
		WriteFailedException();
}

double ADXL355::ValueToGals(long rawValue, int decimals)
{
    double slider = (decimals > 1)? pow(10.0, (double)decimals) : 1.0;

    double result = (double)rawValue / 260000.0 * 980.665;

    result = round(result * slider) / slider;

    return result;
}

void ADXL355::CheckConnection()
{
	if (_fd == -1)
		throw ADXL355_connection_error();
}

void ADXL355::WriteFailedException()
{
	throw ADXL355_io_error();
}

void ADXL355::ErrorIfRunning()
{
	if (IsRunning())
		throw ADXL355_logic_error();
}

long ADXL355::TwosCompliment(unsigned long value)
{
    if (value & (1 << 20 - 1))
        value = value - (1 << 20);

    return value;
}

uint8_t ADXL355::Read8(uint8_t reg)
{
    return wiringPiI2CReadReg8(_fd, reg);
}


uint16_t ADXL355::Read16(uint8_t reg)
{
    return wiringPiI2CReadReg16(_fd, reg);
}

int ADXL355::Write8(uint8_t reg, uint8_t value)
{
    return wiringPiI2CWriteReg8(_fd, reg, value);
}

int ADXL355::Write16(uint8_t reg, uint16_t value)
{
    return wiringPiI2CWriteReg16(_fd, reg, value);
}

int ADXL355::ReadBlock(uint8_t reg, int length, uint8_t *output)
{
    if (length > I2C_SMBUS_BLOCK_MAX)
        return -1;
    
    union i2c_smbus_data data;

    data.block[0] = length;
    data.block[1] = 0;

    struct i2c_smbus_ioctl_data args = { 1, reg, 8, &data };

    int result = ioctl(_fd, I2C_SMBUS, &args);

    if (result == 0)
        memcpy(output, data.block + 1, length);

    return result;
}
