#include <stdint.h>

extern void sensor_setup(uint32_t, uint32_t);
extern uint32_t sensor_read(void);

typedef enum {
	SYMEX_ERROR,
	SYMEX_EXIT,
} ctrl_t;

#define MY_ASSERT(COND) \
	((COND) ? (void)0 : symex_ctrl(SYMEX_ERROR))

static void
symex_ctrl(ctrl_t ctrl)
{
	static volatile uint32_t* const SYMCTRL_CTRL = (uint32_t*)0x02020008;
	uint32_t v;

	switch (ctrl) {
	case SYMEX_ERROR:
		v = (1 << 31);
		break;
	case SYMEX_EXIT:
		v = (1 << 30);
		break;
	}

	*SYMCTRL_CTRL = v;
}

void
symex_exit(void)
{
	symex_ctrl(SYMEX_EXIT);
}

int
main(void)
{
	uint32_t value;

	sensor_setup(23, 42);
	value = sensor_read();

	// This assertion is incorrect since the lower bound is
	// inclusive. That is, there is a path where the sensor
	// returns value 23. This path triggers the assertion.
	// As such, symex-vp will find two paths through this
	// program one where the assertion fails and one where it
	// doesn't fail.
	MY_ASSERT(value > 23 && value < 42);

	return 0;
}
