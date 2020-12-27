#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace std;

pid_t popen2(const char *command, int * infp, int * outfp)
{
	int p_stdin[2], p_stdout[2];
	pid_t pid;

	if (pipe(p_stdin) == -1)
		return -1;

	if (pipe(p_stdout) == -1) {
		close(p_stdin[0]);
		close(p_stdin[1]);
		return -1;
	}

	pid = fork();

	if (pid == 0) {
//		close(p_stdin[1]);
		dup2(p_stdin[0], 0);

//		close(p_stdout[0]);
		dup2(p_stdout[1], 1);

		execl("/bin/sh", "sh", "-c", command, NULL);
		_exit(1);
	}

	close(p_stdin[0]);
	close(p_stdout[1]);

	*infp = p_stdin[1];
	*outfp = p_stdout[0];

	return pid;
}

int main()
{
	int child_stdin,child_stdout = -1;
	pid_t child_pid = popen2("ls", &child_stdin, &child_stdout);

	if(child_pid==-1)return -1;

	char buff[36];
	read(child_stdout, buff, 36);
	cout << "DATA=" << buff << endl;


	return 0;
}
