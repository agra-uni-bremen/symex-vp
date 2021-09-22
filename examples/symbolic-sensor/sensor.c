#include <stdint.h>

static volatile uint32_t* const SENSOR_LOWER_REG = (uint32_t*)0x02020100;
static volatile uint32_t* const SENSOR_UPPER_REG = (uint32_t*)0x02020104;
static volatile uint32_t* const SENSOR_VALUE_REG = (uint32_t*)0x02020108;

void
sensor_setup(uint32_t lower, uint32_t upper)
{
	*SENSOR_LOWER_REG = lower;
	*SENSOR_UPPER_REG = upper;
}

uint32_t
sensor_read(void)
{
	return *SENSOR_VALUE_REG;
}
