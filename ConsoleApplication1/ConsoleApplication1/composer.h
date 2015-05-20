// composer.h: Jarvis Prestidge.
// Description: The class for a Composer record.

#ifndef H_COMPOSER
#define H_COMPOSER

#include <string>

// As we are using a specific library from the stl and not the 
// complete namespace this is acceptable.
using std::string;

// The default ranking is 10 which is the lowest possible
const int kDefaultRanking = 10;

class Composer {
 public:
  // Constructor
  Composer();
  // Here is the destructor which has the same name as the class
  // and is preceded by ~. It is called when an object is destroyed
  // either by deletion, or when the object is on the stack and
  // the method ends.
  ~Composer();
 
  // Accessors and Mutators
  void set_first_name(string in_first_name);
  string first_name();
  void set_last_name(string in_last_name);
  string last_name();
  void set_composer_yob(int in_composer_yob);
  int composer_yob();
  void set_composer_genre(string in_composer_genre);
  string composer_genre();
  void set_ranking(int in_ranking);
  int ranking();
  void set_fact(string in_fact);
  string fact();

  // Methods
  // This method increases a composer's rank by increment.
  void Promote(int increment);
  // This method decreases a composer's rank by decrement.
  void Demote(int decrement);
  // This method displays all the attributes of a composer.
  void Display();

 private:
  string first_name_;
  string last_name_;
  string composer_genre_; // baroque, classical, romantic, etc.
  string fact_;
  int composer_yob_; // year of birth
  int ranking_;
};

#endif