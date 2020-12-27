#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

void createPipe(bool child, int fd[2]){
	if(child)write(fd[1], "hey", 3);
	else{
		sleep(1);
		char buf[6];
		read(fd[0], buf, 6);
		cout << "READ=" << buf << endl;
	}

}

int main()
{
	int fd[2];
	pipe(fd);

	pid_t pid = fork();
	if(pid==0){
		fork();
		createPipe(true, fd);
	}else{
		createPipe(false, fd);
	}
}