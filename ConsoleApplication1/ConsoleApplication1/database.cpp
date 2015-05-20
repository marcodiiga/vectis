// databade.cpp: Jarvis Prestidge.
// Description: Composer database implementation.

#include <iostream>
#include "database.h"

using namespace std;

// Default constructor
Database::Database() {
	// Initialise the next_slot_ member variable integer
	next_slot_ = 0;
}

// Default destructor
Database::~Database() {
}

// Add a new composer using operations in the Composer class.
// For convenience, we return a reference (pointer) to the new record.
Composer& Database::AddComposer(string in_first_name, 
                                string in_last_name,
																string in_genre, 
																int in_yob, 
																string in_fact) {
	// Creating a new composer object
	Composer composer;

  composer.set_first_name(in_first_name);
  composer.set_last_name(in_last_name);
  composer.set_composer_yob(in_yob);
  composer.set_composer_genre(in_genre);
  composer.set_fact(in_fact);

  // Adding the newly created composer object to the composers_ array
  composers_[next_slot_] = composer;

  // Increment the next_slot_ counter
  next_slot_ ++;

  return composers_[(next_slot_-1)];
}

// Search for a composer based on last name. Return a reference to the
// found record.
Composer& Database::GetComposer(string in_last_name) {
	// Iterating through the composer_ array 
	for (int i = 0; i < next_slot_; ++i) {

		if (composers_[i].last_name() == in_last_name) {
			// Return the reference to the matching composer object
			return composers_[i];
		}
	}
	return composers_[0];
}

// Display all composers in the database.
void Database::DisplayAll() {
	for (int i = 0; i < next_slot_; ++i) {
		// Print the names of the composers
		cout << "#" << (i+1) << "\t" << composers_[i].first_name() << endl;
	}
}

// Sort database records by rank and then display all.
void Database::DisplayByRank() {

}