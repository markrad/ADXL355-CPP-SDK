#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstring>
#include <stdexcept>

#include "ADXL355.h"

using namespace std;

void printAxes(int count, long x, long y, long z);
void calibrateSensor(ADXL355 &sensor, int fifoReadCount);

int main()
{
    cout << "Starting sensor test" << endl;

	ADXL355 sensor(0x1d);
	
	try
	{
		int i;
		int result;

		cout << "Analog Devices ID = "  << sensor.GetAnalogDevicesID() << endl;             // Always 173
		cout << "Analog Devices MEMS ID = "  << sensor.GetAnalogDevicesMEMSID() << endl;    // Always 29
		cout << "Device ID = "  << sensor.GetDeviceId() << endl;                            // Always 237 (355 octal)
		cout << "Revision = "  << sensor.GetRevision() << endl;

		sensor.Stop();

		if (sensor.IsI2CSpeedFast())
		{
			cout << "I2C is fast - changing" << endl;
			sensor.SetI2CSpeed(false);
			cout << "Speed is now " << sensor.IsI2CSpeedFast() << endl;
			cout << "Changing back" << endl;
			sensor.SetI2CSpeed(true);
			cout << "Speed is now " << sensor.IsI2CSpeedFast() << endl;
		}
		else
		{
			cout << "I2C is slow - changing" << endl;
			sensor.SetI2CSpeed(true);
			cout << "Speed is now " << sensor.IsI2CSpeedFast() << endl;
			cout << "Changing back" << endl;
			sensor.SetI2CSpeed(false);
			cout << "Speed is now " << sensor.IsI2CSpeedFast() << endl;
		}

		ADXL355::RANGE_VALUES value = sensor.GetRange();

		switch (value)
		{
		case ADXL355::RANGE_VALUES::RANGE_2G:
			cout << "Range 2g" << endl;
			break;
		case ADXL355::RANGE_VALUES::RANGE_4G:
			cout << "Range 4g" << endl;
			break;
		case ADXL355::RANGE_VALUES::RANGE_8G:
			cout << "Range 8g" << endl;
			break;
		default:
			cout << "Unknown range" << endl;
			break;
		}

		ADXL355:sensor.SetRange(ADXL355::RANGE_VALUES::RANGE_8G);

		ADXL355::RANGE_VALUES value2 = sensor.GetRange();

		switch (value2)
		{
		case ADXL355::RANGE_VALUES::RANGE_2G:
			cout << "Range 2g" << endl;
			break;
		case ADXL355::RANGE_VALUES::RANGE_4G:
			cout << "Range 4g" << endl;
			break;
		case ADXL355::RANGE_VALUES::RANGE_8G:
			cout << "Range 8g" << endl;
			break;
		default:
			cout << "Unknown range" << endl;
			break;
		}

		sensor.SetRange(ADXL355::RANGE_VALUES::RANGE_2G);
		sensor.SetOdrLpf(ADXL355::ODR_LPF::ODR_31_25_AND_7_813);
		cout << "Low pass filter = " << sensor.GetOdrLpf();

		ADXL355::HPF_CORNER hpfCorner = sensor.GetHpfCorner();

		cout << "hpfCorner = " << hpfCorner << endl;

		ADXL355::ODR_LPF OdrLpf = sensor.GetOdrLpf();

		cout << "OdrLpf = " << OdrLpf << endl;
		
		sensor.SetTrim(0, 0, 0);

		int32_t xTrim;
		int32_t yTrim;
		int32_t zTrim;

		result = sensor.GetTrim(&xTrim, &yTrim, &zTrim);

		if (result == 0)
		{
			cout << "xTrim=" << xTrim << ";yTrim=" << yTrim << ";zTrim=" << zTrim << endl;
		}

		if (sensor.IsRunning())
		{
			cout << "Sensor is running" << endl;
		}
		else
		{
			cout << "Sensor is not running" << endl;
			cout << "Starting sensor" << endl;
			sensor.Start();

			if (!sensor.IsRunning())
			{
				cout << "Failed to start sensor" << endl;
				return 4;
			}
		}

		if (!sensor.IsTempSensorOn())
		{
			sensor.StartTempSensor();

			if (!sensor.IsTempSensorOn())
			{
				cout << "Failed to start temperature sensor" << endl;
				sensor.Stop();
				return 4;
			}
		}

		this_thread::sleep_for(chrono::milliseconds(1000));

		double temp = sensor.GetTemperatureC();

		cout << "Temperature C is " << temp << endl;

		temp = sensor.GetTemperatureF();

		cout << "Temperature F is " << temp << endl;

		for (i = 0; i < 5; i++)
		{
			cout << "Loop count " << i << " ";
			ADXL355::STATUS_VALUES status = sensor.GetStatus();

			if (status & ADXL355::STATUS_VALUES::NVM_BUSY)
				cout << "NVM_BUSY ";

			if (status & ADXL355::STATUS_VALUES::ACTIVITY)
				cout << "ACTIVITY ";

			if (status & ADXL355::STATUS_VALUES::FIFO_OVERRUN)
				cout << "FIFO_OVERRUN ";

			if (status & ADXL355::STATUS_VALUES::FIFO_FULL)
				cout << "FIFO_FULL ";

			if (status & ADXL355::STATUS_VALUES::DATA_READY)
				cout << "DATA_READY ";

			cout << endl;

			this_thread::sleep_for(chrono::milliseconds(100));
		}

		int fifoCount = sensor.GetFifoCount();

		cout << "FIFO count is " << fifoCount << endl;

		long x;
		long y;
		long z;

		for (i = 0; i < 30; i++)
		{
			if (0 ==sensor.GetRawAxes(&x, &y, &z))
			{
				printAxes(i, x, y, z);
			}
			else
			{
				cout << "Reading failed" << endl;
				break;
			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}

		calibrateSensor(sensor, 5);

		long fifoOut[32][3];

		memset(fifoOut, 0, sizeof(fifoOut));

		while (!(sensor.IsFifoFull()))
		{
			this_thread::sleep_for(chrono::milliseconds(5));
		}

		if (-1 != (result = sensor.ReadFifoEntries((long *)fifoOut)))
		{
			cout << "Retrieved " << result << " entries" << endl;

			for (i = 0; i < result; i++)
			{
				printAxes(i, fifoOut[i][0], fifoOut[i][1], fifoOut[i][2]);
			}
		}

		int count = 0;

		while (!(sensor.GetStatus() & ADXL355::STATUS_VALUES::FIFO_FULL))
		{
			count++;
			this_thread::sleep_for(chrono::milliseconds(5));
		}

		cout << "Looped " << count << " times" << endl;

		sensor.StopTempSensor();

		if (sensor.IsTempSensorOn())
		{
			cout << "Failed to stop temperature sensor" << endl;
		}

		cout << "Stopping sensor" << endl;
		sensor.Stop();

		if (!sensor.IsRunning())
		{
			cout << "Sensor is not running" << endl;
		}
		else
		{
			cout << "Sensor is STILL running" << endl;
		}

		cout << "Exception test" << endl;
		sensor.Start();
		sensor.SetTrim(0, 0, 0);
	}
	catch (ADXL355_connection_error &e)
	{
		cerr << "Connection error thrown: " << e.what() << endl;
		// Pretty much done at this point
	}
	catch (ADXL355_io_error &e)
	{
		cerr << "I/O error thrown: " << e.what() << endl;
		// I guess this could be transient...
	}
	catch (ADXL355_logic_error &e)
	{
		cerr << "Logic error: " << e.what() << endl;
		sensor.Stop();
	}
	catch (exception &e)
	{
		cerr << "Unexpected exception thrown: " << e.what() << endl;
		cerr << "Type: " << typeid(e).name() << endl;
	}
}

void printAxes(int count, long x, long y, long z)
{
    cout << fixed << showpoint << setprecision(4);
    cout
        << "Reading "
        << count
        << "; x=("
        << x
        << ")"
        << ADXL355::ValueToGals(x)
        << "; y=("
        << y
        << ")"
        << ADXL355::ValueToGals(y)
        << "; z=("
        << z
        << ")"
        << ADXL355::ValueToGals(z)
        << endl;
}

void calibrateSensor(ADXL355 &sensor, int fifoReadCount)
{
	long fifoOut[32][3];
	int result;
	int readings = 0;
	long totalx = 0;
	long totaly = 0;
	long totalz = 0;

	memset(fifoOut, 0, sizeof(fifoOut));

	cout << endl << "Calibrating device with " << fifoReadCount << " fifo reads" << endl << endl;

	sensor.Stop();
	sensor.SetTrim(0, 0, 0);
	sensor.Start();
	this_thread::sleep_for(chrono::milliseconds(2000));

	for (int j = 0; j < fifoReadCount; j++)
	{
		cout << "Fifo read number " << j + 1 << endl;

		while (!sensor.IsFifoFull())
		{
			this_thread::sleep_for(chrono::milliseconds(10));
		}

		if (-1 != (result = sensor.ReadFifoEntries((long *)fifoOut)))
		{
			cout << "Retrieved " << result << " entries" << endl;
			readings += result;

			for (int i = 0; i < result; i++)
			{
				totalx += fifoOut[i][0];
				totaly += fifoOut[i][1];
				totalz += fifoOut[i][2];
			}
		}
		else
		{
			cerr << "Fifo read failed" << endl;
		}
	}

	long avgx = totalx / readings;
	long avgy = totaly / readings;
	long avgz = totalz / readings;

	cout
		<< endl
		<< "Total/Average X=" << totalx << "/" << avgx
		<< "; Y=" << totaly << "/" << avgy
		<< "; Z=" << totalz << "/" << avgz
		<< endl << endl;

	sensor.Stop();
	sensor.SetTrim(avgx, avgy, avgz);

	int32_t xTrim;
	int32_t yTrim;
	int32_t zTrim;

	result = sensor.GetTrim(&xTrim, &yTrim, &zTrim);

	if (result == 0)
	{
		cout << "xTrim=" << xTrim << ";yTrim=" << yTrim << ";zTrim=" << zTrim << endl;
	}

	sensor.Start();
	this_thread::sleep_for(chrono::milliseconds(2000));
}