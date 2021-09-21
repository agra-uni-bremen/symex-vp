#include <stdint.h>
#include <stddef.h>

static volatile uint32_t* const SYMCTRL_ADDR = (uint32_t*)0x02020000;
static volatile uint32_t* const SYMCTRL_SIZE = (uint32_t*)0x02020004;
static volatile uint32_t* const SYMCTRL_CTRL = (uint32_t*)0x02020008;

#define SYMEX_ERROR (1 << 31);
#define SYMEX_EXIT  (1 << 30);

void
make_symbolic(void *ptr, size_t size)
{
	*SYMCTRL_ADDR = (uintptr_t)ptr;
	*SYMCTRL_SIZE = size;
}

void
symex_error(void)
{
	*SYMCTRL_CTRL = SYMEX_ERROR;
}

void
symex_exit(void)
{
	*SYMCTRL_CTRL = SYMEX_EXIT;
}
