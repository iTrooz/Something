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
#include <proc_service.h>

using namespace std;


int main(int argc, char **argv) {
	/* After struggled a long time, I got a true way to make my ptrace worked
 * correct with multi-thread application. Here're my sample codes, hope it
 * can help others whom have the same confusion. */

	char trapCode[] = {0, 0, 0, 0};
	int status;


	pid_t childProcess = fork();
	if(childProcess==0){
		char* a[] = {"./threaded", NULL};
		execvp("./threaded", a);
	}

	ptrace(PTRACE_ATTACH, childProcess, NULL, NULL); //childProcess is the main thread
	wait(NULL);

	printf("\nchild %d created\n", childProcess);
	fflush(stdout);

	long ptraceOption = PTRACE_O_TRACECLONE;
	ptrace(PTRACE_SETOPTIONS, childProcess, NULL, ptraceOption);

	struct user_regs_struct regs;

	std::set<pid_t> allThreads;
	std::set<pid_t>::iterator allThreadsIter;
	allThreads.insert(childProcess);

	int rec = ptrace(PTRACE_CONT, childProcess, NULL, NULL);

	while(true)
	{
		pid_t child_waited = waitpid(-1, &status, __WALL);

		if(child_waited == -1)
			break;

		if(allThreads.find(child_waited) == allThreads.end())
		{
			printf("\nreceived unknown child %d\t", child_waited);
			break;
		}

		if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP)
		{

			cout << "hey" << endl;
			int stopCode = WSTOPSIG(status);
			if(stopCode == SIGTRAP)
			{
				int syscall = ptrace(PTRACE_PEEKUSER, child_waited, sizeof(long)*RAX);
				cout << "child " << child_waited << " called " << syscall << " ---------" << endl;
			}

			pid_t new_child;
			if(((status >> 16) & 0xffff) == PTRACE_EVENT_CLONE)
			{
				if(ptrace(PTRACE_GETEVENTMSG, child_waited, 0, &new_child) != -1)
				{
					allThreads.insert(new_child);
					ptrace(PTRACE_CONT, new_child, 0, 0);

					printf("\nchild %d created\t", new_child);
				}

				ptrace(PTRACE_CONT, child_waited, 0, 0);
				continue;
			}
		}

		if(WIFEXITED(status))
		{
			allThreads.erase(child_waited);
			printf("\nchild %d exited with status %d\t", child_waited, WEXITSTATUS(status));

			if(allThreads.size() == 0)
				break;
		}
		else if(WIFSIGNALED(status))
		{
			allThreads.erase(child_waited);
			printf("\nchild %d killed by signal %d\t", child_waited, WTERMSIG(status));

			if(allThreads.size() == 0)
				break;
		}
		else if(WIFSTOPPED(status))
		{
		}

		rec = ptrace(PTRACE_CONT, child_waited, 1, NULL);

		continue;
	}
}
