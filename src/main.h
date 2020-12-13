#include<vector>
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


int do_child(int argc, char **argv);
int do_trace();
int wait_for_syscall(pid_t child);


int maina(int argc, char **argv) {
	return do_trace();
}

int do_child(int argc, char **argv) {
	char *args [argc+1];
	memcpy(args, argv, argc * sizeof(char*));
	args[argc] = NULL;
	ptrace(PTRACE_TRACEME);
	kill(getpid(), SIGSTOP);
	return execvp(args[0], args);
}

int do_trace() {
	pid_t child = 6926;
	int temp;
	temp = ptrace(PTRACE_ATTACH, child, 0, 0);
	if(temp!=0){
		cout << "ptrace failed : code " << temp << endl;
		return 1;
	}
	waitpid(child, &temp, 0);
	ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);
	while(1) {
		cout << "-----------" << endl;
		if (wait_for_syscall(child) != 0) break;
		temp = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX);
		fprintf(stderr, "syscall(%d) = ", temp); // syscall
		if (wait_for_syscall(child) != 0) break;
		temp = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX);
		fprintf(stderr, "%d\n", temp); // retval
	}
	return 0;
}

int wait_for_syscall(pid_t child) {
	int status;
	while (true) {
		ptrace(PTRACE_SYSCALL, child, 0, 0);
		waitpid(child, &status, 0);
		cout << WIFSTOPPED(status) << endl;
		if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80)
			return 0;
		if (WIFEXITED(status))
			return 1;
	}
}
