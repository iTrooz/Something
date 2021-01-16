#include <iostream>
#include <fstream>
using namespace std;

int main(){
	fstream my_file;
	my_file.open("my_file", ios::out);
	if (!my_file) {
		cout << "File not created!" << endl;
	}
	else {
		my_file << "salut" << endl;
		cout << "File created successfully!" << endl;
		my_file.close();
	}
	return 0;
}