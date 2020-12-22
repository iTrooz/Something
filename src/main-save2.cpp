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
int do_trace(pid_t child);
int wait_for_syscall(pid_t child);

int main(int argc, char **argv) {
	char* a[] = {"./Test", "ls", NULL};
	argc = 2;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s prog args\n", argv[0]);
        exit(1);
    }

//    pid_t child = fork();
//    if (child == 0) {
//        return do_child(argc-1, argv+1);
//    } else {
//        return do_trace(child);
//    }
	do_trace(NULL);
}

int do_child(int argc, char **argv) {
    char *args [argc+1];
    memcpy(args, argv, argc * sizeof(char*));
    args[argc] = NULL;
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
}

int do_trace(pid_t child) {

	child = fork();
	if(child==0){
		char* a[] = {"ls", NULL};
		execvp("ls", a);
	}

    int status, syscall, retval;
	status = ptrace(PTRACE_ATTACH, child, NULL, NULL); //childProcess is the main thread
//    waitpid(child, &status, 0);
	wait(NULL);
	printf("\nchild %d created\n", child);

    status = ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACESYSGOOD|PTRACE_O_TRACECLONE);
//    kill(child, SIGCONT);
	ptrace(PTRACE_CONT, child, NULL, NULL);

    while(1) {
//		cout << "trying" << endl;
//		waitpid(-1, &status, __WALL);
//		cout << "OK" << endl;
//		return 0;
		int a = wait_for_syscall(child);
        if (a != 0){
        	cout << "br " << a << endl;
        	break;
        };
        syscall = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX);
        cout << "CALL=" << syscall << endl;
//        fprintf(stderr, "syscall(%d) = ", syscall);
        if (wait_for_syscall(child) != 0) break;
        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*RAX);
        fprintf(stderr, "%d\n", retval);
    }
    cout << "out of loop" << endl;
    return 0;
}

int wait_for_syscall(pid_t child) {
    int status;
    while (1) {
        ptrace(PTRACE_SYSCALL, child, 0, 0);
//        ptrace(PTRACE_SYSCALL, child, 0, 0);
        cout << "a" << endl;
        waitpid(-1, &status, __WALL);
        cout << "b" << endl;
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80){
            return 0;
        }
        if (WIFEXITED(status)){
        	cout << "NOP" << endl;
        	return 1;
        }
    }
}
