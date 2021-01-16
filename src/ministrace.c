#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int do_child(int argc, char **argv);
int do_trace(pid_t mainProcess);

int wait_for_syscall(pid_t* stopped) {
	int status;
	while (1) {
		*stopped = waitpid(-1, &status, 0);
		if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80){
			return 0;
		}
		if (WIFEXITED(status)){
			return 1;
		}
		ptrace(PTRACE_SYSCALL, *stopped, 0, 0);
	}
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s prog args\n", argv[0]);
        exit(1);
    }

    pid_t child = fork();
    if (child == 0) {
        return do_child(argc-1, argv+1);
    } else {
        return do_trace(child);
    }
}

int do_child(int argc, char **argv) {
    char *args [argc+1];
    memcpy(args, argv, argc * sizeof(char*));
    args[argc] = NULL;
    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
}

int do_trace(pid_t mainProcess) {
    int status, syscall, retval;
    waitpid(mainProcess, &status, 0);
//    ptrace(PTRACE_SETOPTIONS, mainProcess, 0, PTRACE_O_TRACESYSGOOD);
    ptrace(PTRACE_SETOPTIONS, mainProcess, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT);
	ptrace(PTRACE_SYSCALL, mainProcess, 0, 0);

    int stopped;
    while(1) {
        if (wait_for_syscall(&stopped) != 0){
			if(stopped==mainProcess)break;
        }

        // Interpret call

		ptrace(PTRACE_SYSCALL, stopped, 0, 0);
    }
    return 0;
}