#ifndef _ADXL355_H
#define _ADXL355_H

#include <stdint.h>

#include "ADXL355_connection_error.h"
#include "ADXL355_io_error.h"
#include "ADXL355_logic_error.h"

class ADXL355
{
private:
    static const uint8_t DEVID_AD = 0x00;
    static const uint8_t DEVID_MST = 0x01;
    static const uint8_t PARTID = 0x02;
    static const uint8_t REVID = 0x03;
    static const uint8_t STATUS = 0x04;
    static const uint8_t FIFO_ENTRIES = 0x05;
    static const uint8_t I2CSPEED_INTPOLARITY_RANGE = 0x2c;
    static const uint8_t POWER_CTL = 0x2d;
    static const uint8_t RESET = 0x2f;

    static const uint8_t RESET_VALUE = 0x52;

    static const uint8_t TEMP2 = 0x06;
    static const uint8_t TEMP1 = 0x07;

    static const uint8_t XDATA3 = 0x08;
    static const uint8_t XDATA2 = 0x09;
    static const uint8_t XDATA1 = 0x0a;
    static const uint8_t YDATA3 = 0x0b;
    static const uint8_t YDATA2 = 0x0c;
    static const uint8_t YDATA1 = 0x0d;
    static const uint8_t ZDATA3 = 0x0e;
    static const uint8_t ZDATA2 = 0x0f;
    static const uint8_t ZDATA1 = 0x10;
    
    static const uint8_t FIFO_DATA = 0x11;

    static const uint8_t OFFSET_X_H = 0x1e;
    static const uint8_t OFFSET_X_L = 0x1f;
    static const uint8_t OFFSET_Y_H = 0x20;
    static const uint8_t OFFSET_Y_L = 0x21;
    static const uint8_t OFFSET_Z_H = 0x22;
    static const uint8_t OFFSET_Z_L = 0x23;

    static const uint8_t FILTER = 0x28;

    enum POWER_CTL_VALUES
    {
        POWER_CTL_OFF = 0x01,
        POWER_CTL_ON = ~POWER_CTL_OFF,
        POWER_CTL_TEMP_OFF = 0x02,
        POWER_CTL_TEMP_ON = ~POWER_CTL_TEMP_OFF
    };
	
	enum I2C_SPEED_VALUES
	{
		I2C_SPEED_FAST = 0x80,
		I2C_SPEED_SLOW = 0x00
	};

    int _deviceId;
    int _fd;

public:

    enum STATUS_VALUES
    {
        NVM_BUSY = 0x10,
        ACTIVITY = 0x08,
        FIFO_OVERRUN = 0x04,
        FIFO_FULL = 0x02,
        DATA_READY = 0x01
    };

    enum RANGE_VALUES
    {
        RANGE_2G = 0x01,
        RANGE_4G = 0x02,
        RANGE_8G = 0x03,
        RANGE_MASK = 0x03
    };

    enum HPF_CORNER
    {
        DISABLED = 0b000,
        ODR_X_247 = 0b001,
        ODR_X_62_084 = 0b010,
        ODR_X_15_545 = 0b011,
        ODR_X_3_862 = 0b100,
        ODR_X_0_954 = 0b101,
        ODR_X_0_238 = 0b110,
        HPF_CORNER_MASK = 0b01110000
    };

    enum ODR_LPF
    {
        ODR_4000_AND_1000 = 0b0000,
        ODR_2000_AND_500 = 0b0001,
        ODR_1000_AND_250 = 0b0010,
        ODR_500_AND_125 = 0b0011,
        ODR_250_AND_62_5 = 0b0100,
        ODR_125_AND_31_25 = 0b0101,
        ODR_62_5_AND_15_625 = 0b0110,
        ODR_31_25_AND_7_813 = 0b0111,
        ODR_15_625_AND_3_906 = 0b1000,
        ODR_7_813_AND_1_953 = 0b1001,
        ODR_3_906_AND_0_977 = 0b1010,
        ODR_LPF_MASK = 0b1111
    };

    ADXL355(int devId);
    ~ADXL355();

    uint8_t GetAnalogDevicesID();
    uint8_t GetAnalogDevicesMEMSID();
    uint8_t GetDeviceId();
    uint8_t GetRevision();
    STATUS_VALUES GetStatus();
    bool IsFifoFull();
    bool IsFifoOverrun();
    bool IsDataReady();
    HPF_CORNER GetHpfCorner();
    ODR_LPF GetOdrLpf();
    void SetHpfCorner(HPF_CORNER value);
    void SetOdrLpf(ODR_LPF value);
    int GetFifoCount();

    void Start();
    void Stop();
    bool IsRunning();
    bool IsTempSensorOn();
    void StartTempSensor();
    void StopTempSensor();
    void Reset();
	double GetTemperatureC();
	double GetTemperatureF();
    int GetRawAxes(long *x, long *y, long *z);
    int GetTrim(int32_t *x, int32_t *y, int32_t *z);
    void SetTrim(int32_t x, int32_t y, int32_t z);
	bool IsI2CSpeedFast();
	void SetI2CSpeed(bool value);
    
    // Output must be 96 entries
    int ReadFifoEntries(long *output);

    RANGE_VALUES GetRange();
    void SetRange(RANGE_VALUES value);

    static double ValueToGals(long rawValue, int decimals = 3);

private:
	void CheckConnection();
	void WriteFailedException();
	void ErrorIfRunning();
    long TwosCompliment(unsigned long value);
    uint8_t Read8(uint8_t reg);
    uint16_t Read16(uint8_t reg);
    int ReadBlock(uint8_t reg, int length, uint8_t *output);
    int Write8(uint8_t reg, uint8_t value);
    int Write16(uint8_t reg, uint16_t value);    
};
#endif // ifndef _ADXL355_H