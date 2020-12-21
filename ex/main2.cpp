#include<iostream>
#include<thread>
#include<unistd.h>

using namespace std;

void test(){
	cout << "malicious" << endl;
}

int main(){
	thread thr(test);
	thr.join();
//	if(fork()==0)test();
//	return 0;
}