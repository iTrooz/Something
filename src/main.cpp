#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include<iostream>
#include <cstdlib>
#include <unistd.h>
#include <proc_service.h>

using namespace std;

bool waitProcess(pid_t& stopped) {
	int status;
	while (true) {
		fflush(stdout);
		stopped = waitpid(stopped, &status, 0);
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

	int temp;

	waitpid(mainProcess, &temp, 0);
	temp = ptrace(PTRACE_SETOPTIONS, mainProcess, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE);
	if (temp != 0)throw runtime_error("PTRACE_SETOPTIONS failed : " + to_string(temp));
	temp = ptrace(PTRACE_SYSCALL, mainProcess, 0, 0); // restart le thread + l'arrête au prochain syscall
	if (temp != 0)throw runtime_error("FIRST PTRACE_SYSCALL normal-failed : " +
									  to_string(temp)); // TODO C'est normal si ca throw ici !! enlever throw

	__ptrace_syscall_info info{};
	user_regs_struct regs{};
	int size = sizeof(__ptrace_syscall_info);


	while (true) {
		if (waitProcess(mainProcess))break;
		// entry

		info.op = PTRACE_SYSCALL_INFO_ENTRY;
		ptrace(PTRACE_GET_SYSCALL_INFO, mainProcess, size, &info);
		if(info.entry.nr==1){
//			cout << endl << endl;
//			cout << "call " << info.entry.nr << endl;
//			cout << info.entry.args[0] << endl; // pipe = rbx
//			cout << info.entry.args[1] << endl; // pointer = rcx
//			cout << info.entry.args[2] << endl; // len = rdx
//			cout << "-" << endl;
			ptrace(PTRACE_GETREGS, mainProcess, 0, &regs);
			regs.rdi = 2;
//			cout << "REG = " << regs.rdi << endl;
			ptrace(PTRACE_SETREGS, mainProcess, 0, &regs);
//			temp = ptrace(PTRACE_PEEKUSER, mainProcess, sizeof(long)*RDI); // restart le thread + l'arrête au prochain syscall
//			cout << "PEEK = " << temp << endl;
		}

		/*
		 args[0] = pipe
		 args[1] = pointer = rcx
		 args[2] = size = rbx/rdx ?
		 syscall id = orig_rax
		 */


		temp = ptrace(PTRACE_SYSCALL, mainProcess, 0, 0); // restart le thread + l'arrête au prochain syscall

		if (waitProcess(mainProcess))break;
		// exit

		temp = ptrace(PTRACE_SYSCALL, mainProcess, 0, 0); // restart le thread + l'arrête au prochain syscall
	}
	cout << "CHILD EXITED" << endl;
}

int main(){

	pid_t child = fork();
	if(child==0){
		char* a[] = {"sudo", NULL};
		char** b = a;
		ptrace(PTRACE_TRACEME);
		kill(getpid(), SIGSTOP);
		int t = execvp(b[0], b);
		cout << "AAA="<<t << endl;
		throw runtime_error("NOT SUPPOSED TO HAPPEN : PROCESS ESCAPED"); // au cas ou
	}else {
		startTrace(child);
//		sleep(2);
	}
}
