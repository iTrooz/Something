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
#include <set>

using namespace std;

int do_child(int argc, char **argv);

[[noreturn]] int do_trace(pid_t child);
int wait_for_syscall(pid_t who, pid_t& stopped);
int PRINCIPAL;

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
		PRINCIPAL = child;
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

int do_trace(pid_t tracee) {
    int status, syscall, retval, stopped;
	set<pid_t> traced;
	traced.insert(tracee);

    waitpid(tracee, &status, 0);
    status = ptrace(PTRACE_SETOPTIONS, tracee, 0, PTRACE_O_TRACESYSGOOD|PTRACE_O_TRACECLONE|PTRACE_O_TRACEFORK);
    if(status!=0)cout << "PTRACE_SETOPTIONS failed : " << status << endl;
	status = ptrace(PTRACE_SYSCALL, tracee, 0, 0); // restart le thread + l'arrête au prochain syscall
    if(status!=0)cout << "FIRST PTRACE_SYSCALL failed : " << status << endl;

    while(true) {
    	flush(cout);
    	cout << "------------" << endl;
		cout << "a" << endl;

        if (wait_for_syscall(-1, stopped) != 0) {
        	if(traced.erase(stopped)==0){
        		cout << "Caught invalid child : " << stopped << endl;
			}else{
        		cout << "child " << stopped << " exited" << endl;
        		if(traced.empty())break;
        	}
        }

        syscall = ptrace(PTRACE_PEEKUSER, stopped, sizeof(long)*ORIG_RAX);
		cout << "-- " << syscall << " " << stopped << endl;

		status = ptrace(PTRACE_SYSCALL, stopped, 0, 0); // restart le thread + l'arrête au prochain syscall
		if(status!=0)cout << "failed : " << status << endl;

        if (wait_for_syscall(stopped, stopped) != 0){
			if(traced.erase(stopped)==0){
				cout << "Caught invalid child : " << stopped << endl;
			}else{
				cout << "child " << stopped << " exited" << endl;
				if(traced.empty())break;
			}
        }

        retval = ptrace(PTRACE_PEEKUSER, stopped, sizeof(long)*RAX);
		cout << "---" << endl;
		if(status!=0)cout << "failed : " << status << endl;
    	cout << stopped << "|" << syscall << "->" << retval << endl;

		if(syscall==56) {
			if(status!=0)cout << "failed : " << status << endl;
			pid_t new_child = retval;
			cout << "created subprocess " << new_child << endl;
			int exists = access(("/proc/" + to_string(new_child)).c_str(), 0);
			cout << "exists : " << exists << endl;
			if(exists==0) {
				traced.insert(new_child);
				status = ptrace(PTRACE_SYSCALL, new_child, 0, 0); // restart le thread + l'arrête au prochain syscall
				if(status!=0){
					cout << "failed : " << status << endl;
					cout << explain_ptrace(PTRACE_SYSCALL, new_child, 0, 0) << endl;
				}
			}
		}


		status=ptrace(PTRACE_SYSCALL, stopped, 0, 0); // restart le thread + l'arrête au prochain syscall
		if(status!=0)cout << "failed : " << status << endl;
    }
    cout << "out of loop" << endl;
    return 0;
}

int wait_for_syscall(pid_t who, pid_t& stopped) {
    int status;
    while (true) {
        stopped = waitpid(who, &status, __WALL);

        if(stopped != PRINCIPAL)cout << "OK!!!!!!!!!!!!!!! " << stopped << endl;
		if(((status >> 16) & 0xffff) == PTRACE_EVENT_CLONE){
			cout << "CLONE SIGNAL" << endl;
		}
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80){
            return 0;
        }
        if (WIFEXITED(status)){
        	cout << "exit" << endl;
            return 1;
        }
		status=ptrace(PTRACE_SYSCALL, stopped, 0, 0); // restart le thread + l'arrête au prochain syscall
		if(status!=0)cout << "failed : " << status << endl;
    }
}
