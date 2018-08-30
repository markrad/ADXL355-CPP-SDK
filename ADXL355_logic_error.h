#include <stdexcept>

struct ADXL355_logic_error : public std::logic_error
{
    ADXL355_logic_error() : logic_error("Settings cannot be modified when sensor is running")
    {
    }
};