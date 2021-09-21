#include <stdint.h>
#include <stddef.h>

extern void symex_error(void);
extern void make_symbolic(void *, size_t);

#define MY_ASSERT(COND) \
	((COND) ? (void)0 : symex_error())

int
main(void)
{
	int a;

	make_symbolic(&a, sizeof(a));
	if (a % 2 == 0)
		MY_ASSERT(a != 42);

	return 0;
}
