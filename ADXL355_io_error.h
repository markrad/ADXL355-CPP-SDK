#include <stdexcept>

struct ADXL355_io_error : public std::runtime_error
{
    ADXL355_io_error() : runtime_error("I/O failure reading or writing to the sensor")
    {
        
    }
};