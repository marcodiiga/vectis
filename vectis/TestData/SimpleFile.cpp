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


CUSTOMTYPE function(ANOTHER_CUSTOM_TYPE parm = 22);                            
void anotherFunction();
class forwardMe;

class testClass {
protected:
	testClass();
};

struct CUSTOMTYPE {

};
#define CUSTOMTYPE ea

testClass::testClass() {

}

class ADerived : public testClass {
	testClass * l;
	testClass& l23;
};

DARNIT();

int main();

int main() {
  testClass obj;
  int vardecl = 22;
  return 0;
}