#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

bool waitProcess(pid_t& stopped) {
	int status;
	while (true) {
		stopped = waitpid(-1, &status, __WALL);
		if(stopped==-1){
			exit(0);
		}

		if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80){
			return false;
		}
		if (WIFEXITED(status)){
			return true;
		}
		status = ptrace(PTRACE_SYSCALL, stopped, 0, 0); // restart le thread + l'arrête au prochain syscall
		if(status!=0)cerr << "failed wait_for_syscall : " << status << endl;
	}
}

int startTrace(pid_t mainProcess) {
	long temp;
	int stopped;

	waitpid(mainProcess, nullptr, 0);
	temp = ptrace(PTRACE_SETOPTIONS, mainProcess, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_EVENT_VFORK | PTRACE_O_TRACECLONE);
	if (temp != 0)throw runtime_error("PTRACE_SETOPTIONS failed : " + to_string(temp));
	temp = ptrace(PTRACE_SYSCALL, mainProcess, 0, 0); // restart le thread + l'arrête au prochain syscall
	if (temp != 0)throw runtime_error("FIRST PTRACE_SYSCALL normal-failed : " +
									  to_string(temp)); // TODO C'est normal si ca throw ici !! enlever throw

	while (true) {
		if(waitProcess(stopped))break;
		ptrace(PTRACE_SYSCALL, stopped, 0, 0); // restart le thread + l'arrête au prochain syscall
	}
	cout << "OUT" << endl;
	fflush(stdout);
	return 0;
}

int do_child(int argc, char **argv) {
    char *args [argc+1];
    memcpy(args, argv, argc * sizeof(char*));
    args[argc] = NULL;
    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
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
		return startTrace(child);
	}
}
