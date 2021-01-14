#include <sys/ptrace.h>
#include <sys/wait.h>
#include<iostream>
#include <cstdlib>
#include <unistd.h>
#include <proc_service.h>

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


void startTrace(pid_t mainProcess) { // TODO way to kill tracer ?

	long temp;
	int stopped;

	waitpid(mainProcess, nullptr, 0);
//	temp = ptrace(PTRACE_SETOPTIONS, mainProcess, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE);
	temp = ptrace(PTRACE_SETOPTIONS, mainProcess, 0, PTRACE_O_TRACESYSGOOD);
	if (temp != 0)throw runtime_error("PTRACE_SETOPTIONS failed : " + to_string(temp));
	temp = ptrace(PTRACE_SYSCALL, mainProcess, 0, 0); // restart le thread + l'arrête au prochain syscall
	if (temp != 0)throw runtime_error("FIRST PTRACE_SYSCALL normal-failed : " + to_string(temp));

	__ptrace_syscall_info* info;
	int size = sizeof(__ptrace_syscall_info);
	int procs = 1;

	while (true) {
//		cout << endl << "---CALL---" << endl;
		if(waitProcess(stopped)){
			cout << endl << "---BREAK---" << endl;
			break;
//			if(stopped==mainProcess)break;
//			procs--;
//			if(procs==0)break;
//			else continue;
		}

//		info = new __ptrace_syscall_info();
//		ptrace(PTRACE_GET_SYSCALL_INFO, stopped, size, info);
//		if(info->op==PTRACE_SYSCALL_INFO_ENTRY&& info->entry.nr == 56){
//			procs++;
//			cout << "CLONE" << endl;
//		}
//		delete info;

		ptrace(PTRACE_SYSCALL, stopped, 0, 0); // restart le thread + l'arrête au prochain syscall
	}
	cout << endl << "---PROCESS DEAD---" << endl;
	fflush(stdout);
}

int do_child(int argc, char **argv) {
	char *args [argc+1];
	int i;
	for (i=0;i<argc;i++)
		args[i] = argv[i];
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
	if(child==0){
		do_child(argc-1, argv+1);
		cerr << "NOT SUPPOSED TO HAPPEN : PROCESS ESCAPED" << endl; // au cas ou
		exit(1);
	}else {
		startTrace(child);
	}
}
