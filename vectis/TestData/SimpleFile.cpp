#define MYMACRO test \
				test2
				
/*
  Multiline comment
*/
#include <iostream> // Inline comment
#include "other_header.h"

using namespace something;
using namespace something_else;

#define MYCOMPLEXMACRO(a,b) macro1 \
                            macro2


CUSTOMTYPE function(ANOTHER_CUSTOM_TYPE parm = 22){
	int lol = func();
	int ea = 32ll;
	void myfunc();
	return something(lolz);
}                            
void anotherFunction(float myval = 0x00239FFEDD);
class forwardMe;

struct myStruct {
	char something();
};

class testClass {
protected:
	testClass();
	int lolfunc();
	class innerClass {
		public: coolThings;
	};
};

int testClass::lolfunc() {
	accessoryClass::lol();
}

struct CUSTOMTYPE {

};
#define CUSTOMTYPE ea

testClass::testClass() {

}

class ADerived : public testClass {
	testClass * l;
	testClass& l23;
};

template<typename T, int dd = 23>
class ILoveMosquitoes : private Somethingelse<T> {
	T cool = "hello world";
	T cool2 = 'nope';
};

DARNIT();

int main();

int main() {
  testClass obj;
  int vardecl = 22;
  return 0;
}