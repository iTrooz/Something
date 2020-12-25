#include<iostream>
#include<thread>
#include<unistd.h>

using namespace std;

void test(){
	cout << "malicious" << endl;
	exit(0);
}

int main(){
	thread thr(test);
	thr.detach();
	usleep(1000*500);
}