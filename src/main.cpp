#include "main.h"


class MyType{
public:
	int* value;
	MyType() = default;
	MyType(MyType&& s){
		cout << "MOVE"<< endl;
		value = s.value;
		s.value = nullptr;
	};
	MyType(MyType const &s2){
		cout << "COPY"<< endl;
		value = s2.value;
	};
};

int main(){
//	MyType a;
//
//	int test = 4;

//	a.value = &test;
//	cout << a.value << endl;
//	MyType b = a; // Je déplace le contenu de a vers b
//	cout << b.value << endl;

	string a = "salut";
	string b = "dégage";
	string c = "";
	cout << &a << endl;
	c = move(a);
	cout << &b << endl;
	cout << &c << endl;


//	Test t;
//	test(t);
//	cout << t.value.value << endl;


//	char* a = "ls";
//	char* b = "-al";
//	execvp(a, b);
}
