#include<iostream>
#include<thread>
#include<unistd.h>

using namespace std;

void test(){
	cout << "malicious" << endl;
}

int main(){
	thread thr(test);
	thr.detach();
	int k=0;
	for(int i=0;i<1000;i++){
		for(int j=0;j<1000;j++){
			k+=(i-j);
		}
	}
	cout << "fin first" << endl;
//	if(fork()==0)test();
	return 0;
}