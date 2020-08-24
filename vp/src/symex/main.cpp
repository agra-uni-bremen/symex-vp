#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <systemc.h>

int
main(int argc, char **argv)
{
	pid_t pid;
	int wstatus, ret;

	switch ((pid = fork())) {
	case -1:
		err(EXIT_FAILURE, "fork failed");
		break;
	case 0:
		return sc_core::sc_elab_and_sim(argc, argv);
		break;
	default:
		if (waitpid(pid, &wstatus, 0) == -1)
			err(EXIT_FAILURE, "waitpid failed");

		ret = WEXITSTATUS(wstatus);
		printf("Child %u exited with status %d\n", (unsigned)pid, ret);
	}

	return 0;
}
