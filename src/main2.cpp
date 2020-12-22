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
#include <thread>

#include <libexplain/ptrace.h>
#include <vector>

using namespace std;

int do_child(int argc, char **argv);

[[noreturn]] int do_trace(pid_t child);
int wait_for_syscall(pid_t child);

int main(int argc, char **argv) {
	char* a[] = {"./Test", "./threaded", NULL};
	argv = a;
	argc = 2;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s prog args\n", argv[0]);
        exit(1);
    }

	cout << "current pid : " << getpid() << endl;
    pid_t child = fork();
    if (child == 0) {
        return do_child(argc-1, argv+1);
    } else {
		cout << "child pid : " << child << endl;
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

[[noreturn]] int do_trace(pid_t child) {
    int status, syscall, retval;
    waitpid(child, &status, 0);
    status = ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);
    if(status!=0)cout << "failed : " << status << endl;
    int i=0;
    while(true) {
    	flush(cout);
    	cout << endl;
		cout << "a" << endl;
        if (wait_for_syscall(child) != 0) break;
        syscall = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX);
    	cout << "b" << endl;
        if (wait_for_syscall(child) != 0) break;
    	cout << "c" << endl;
        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX);
    	cout << syscall << endl;

		if(syscall==56){
			cout << "created subprocess " << retval << endl;
			cout << "exists : " << access(("/proc/"+to_string(retval)).c_str(), 0) << endl;

//			status = ptrace(PTRACE_ATTACH, retval, 0, 0);
//			if(status!=0)cout << "PTRACE_ATTACH failed on " << retval << " : " << status << endl;
//			kill(retval, SIGSTOP);
//			waitpid(retval, &status, 0);
//			status = ptrace(PTRACE_SETOPTIONS, retval, 0, PTRACE_O_TRACESYSGOOD);
//			if(status!=0)cout << "PTRACE_SETOPTIONS failed on " << retval << " : " << status << endl;
//			else cout << "ptraced child " << retval << endl;
//
////			string s = explain_ptrace(PTRACE_SETOPTIONS, retval, 0, (void *) PTRACE_O_TRACESYSGOOD);
////			cout << s << endl;
//
		}
		i++;
    }
    return 0;
}

int wait_for_syscall(pid_t child) {
    int status;
    while (true) {
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        int a = waitpid(-1, &status, __WALL);
        if(a!=child)cout << "OK!!!!!!!!!!!!!!! " << a << endl;
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80){
            return 0;
        }if (WIFEXITED(status)){
        	cout << "exit" << endl;
            return 1;
        }
    }
}
