// test_database.cpp: Jarvis Prestidge.
// Description: Test driver for a database of Composer records.

#include <iostream>
#include "Database.h"
using namespace std;

int main() {
  Database myDB;

  // Remember that AddComposer returns a reference to the new record.
  Composer& comp1 = myDB.AddComposer("Ludwig van", "Beethoven", "Romantic", 1770,
    "Beethoven was completely deaf during the latter part of his life - he never "
    "heard a performance of his 9th symphony.");
  comp1.Promote(7);

  Composer& comp2 = myDB.AddComposer("Johann Sebastian", "Bach", "Baroque", 1685,
    "Bach had 20 children, several of whom became famous musicians as well.");
  comp2.Promote(5);

  Composer& comp3 = myDB.AddComposer("Wolfgang Amadeus", "Mozart", "Classical", 1756,
    "Mozart feared for his life during his last year - there is some evidence "
    "that he was poisoned.");
  comp3.Promote(2);

  cout << endl << "all Composers: " << endl << endl;
  myDB.DisplayAll();
}