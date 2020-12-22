#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <libexplain/ptrace.h>

using namespace std;


int do_child(int argc, char **argv);
int do_trace(pid_t child);
int wait_for_syscall(pid_t child);

int main(int argc, char **argv) {
	char* a[] = {"./Test", "ls", NULL};
	argv = a;
	argc = 2;

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

int do_trace(pid_t child) {
    int status, syscall, retval;
    waitpid(child, &status, 0);
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);

//	int a = ptrace(PTRACE_SYSCALL, child, 0, 0);
//	cout << "RET="<<a<<endl;
//	cout << explain_ptrace(PTRACE_SYSCALL, child, 0, 0) << endl;

	waitpid(child, &status, 0);

	return -1;

    while(1) {
        if (wait_for_syscall(child) != 0) break;
        syscall = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX);
//        fprintf(stderr, "syscall(%d) = ", syscall);
        if (wait_for_syscall(child) != 0) break;
        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX);
//        fprintf(stderr, "%d\n", retval);
    }
    return 0;
}

int wait_for_syscall(pid_t child) {
    int status;
    while (1) {
        int a = ptrace(PTRACE_SYSCALL, child, 0, 0);
		cout << explain_ptrace(PTRACE_SYSCALL, child, 0, 0) << endl;
//        cout << "RET="<<a<<endl;
        waitpid(child, &status, 0);
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80)
            return 0;
        if (WIFEXITED(status))
             return 1;
    }
}
