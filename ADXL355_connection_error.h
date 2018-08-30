#include <stdexcept>

struct ADXL355_connection_error : public std::runtime_error
{
    ADXL355_connection_error() : runtime_error("Failed to connect to ADXL355 sensor")
    {

    }
};