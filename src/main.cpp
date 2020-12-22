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

using namespace std;


pid_t start_child();
int do_trace();
int wait_for_syscall(pid_t child);

int main(int argc, char **argv) {
    do_trace();
    return 0;
}

pid_t start_child() {
    pid_t child = fork();
    if(child==0){
	    kill(getpid(), SIGSTOP);
		char* a[] = {"ls", NULL};
	    execvp("ls", a);
		cout << "ALEEEEEEEEEERTE" << endl;
    }
    return child;
}

int do_trace() {
	cout << "a" << endl;
	pid_t child = start_child();

    int temp, syscall, retval;
	cout << "b" << endl;
	temp = ptrace(PTRACE_ATTACH, child, 0, 0);
	if(temp!=0){
		cout << "PTRACE_ATTACH error : " << temp << endl;
	}
	cout << "c" << endl;
    waitpid(child, &temp, 0);
    temp = ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);
	if(temp!=0){
		cout << "PTRACE_SETOPTIONS error : " << temp << endl;
	}
	cout << "d" << endl;
    while(1) {
        if (wait_for_syscall(child) != 0) break;
        syscall = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX);
        fprintf(stderr, "syscall(%d) = ", syscall);
        if (wait_for_syscall(child) != 0) break;
        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX);
        fprintf(stderr, "%d\n", retval);
    }
    return 0;
}

int wait_for_syscall(pid_t child) {
    int status;
    while (1) {
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        waitpid(child, &status, 0);
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80)
            return 0;
        if (WIFEXITED(status))
             return 1;
    }
}
