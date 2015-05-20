#include "windows.h"
#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <unordered_set>

using namespace std;



std::unordered_set<std::string> set;
void populateKeywords() {
	set.emplace("alignas");
	set.emplace("alignof");
	set.emplace("and");
	set.emplace("and_eq");
	set.emplace("asm");
	set.emplace("auto");
	set.emplace("bitand");
	set.emplace("bitor");
	set.emplace("bool");
	set.emplace("break");
	set.emplace("case");
	set.emplace("catch");
	set.emplace("char");
	set.emplace("char16_t");
	set.emplace("char32_t");
	set.emplace("class");
	set.emplace("compl");
	set.emplace("concept");
	set.emplace("const");
	set.emplace("constexpr");
	set.emplace("const_cast");
	set.emplace("continue");
	set.emplace("decltype");
	set.emplace("default");
	set.emplace("delete");
	set.emplace("do");
	set.emplace("double");
	set.emplace("dynamic_cast");
	set.emplace("else");
	set.emplace("enum");
	set.emplace("explicit");
	set.emplace("export");
	set.emplace("extern");
	set.emplace("false");
	set.emplace("float");
	set.emplace("for");
	set.emplace("friend");
	set.emplace("goto");
	set.emplace("if");
	set.emplace("inline");
	set.emplace("int");
	set.emplace("long");
	set.emplace("mutable");
	set.emplace("namespace");
	set.emplace("new");
	set.emplace("noexcept");
	set.emplace("not");
	set.emplace("not_eq");
	set.emplace("nullptr");
	set.emplace("operator");
	set.emplace("or");
	set.emplace("or_eq");
	set.emplace("private");
	set.emplace("protected");
	set.emplace("public");
	set.emplace("register");
	set.emplace("reinterpret_cast");
	set.emplace("requires");
	set.emplace("return");
	set.emplace("short");
	set.emplace("signed");
	set.emplace("sizeof");
	set.emplace("static");
	set.emplace("static_assert");
	set.emplace("static_cast");
	set.emplace("struct");
	set.emplace("switch");
	set.emplace("template");
	set.emplace("this");
	set.emplace("thread_local");
	set.emplace("throw");
	set.emplace("true");
	set.emplace("try");
	set.emplace("typedef");
	set.emplace("typeid");
	set.emplace("typename");
	set.emplace("union");
	set.emplace("unsigned");
	set.emplace("using");
	set.emplace("virtual");
	set.emplace("void");
	set.emplace("volatile");
	set.emplace("wchar_t");
	set.emplace("while");
	set.emplace("xor");
	set.emplace("xor_eq");
	set.emplace("nullptr");
}

enum LexerScope { GlobalScope, ClassScope, FunctionScope, SingleLineComment, MultiLineComment };
//LexerScope currentScope;

void printToConsole(std::string str, int color) {
	HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	// Remember how things were when we started
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hstdout, &csbi);

	SetConsoleTextAttribute(hstdout, color);
	cout << str;

	FlushConsoleInputBuffer(hstdin);

	// Keep users happy
	SetConsoleTextAttribute(hstdout, csbi.wAttributes);
}



//void continueCurrentScope(std::string& str, size_t& pos);

void lineCommentStatement(std::string& str, size_t& pos) {
	// A statement spans until a newline is found (or EOF)

	size_t start = pos; // DEBUG

	// Skip everything until \n
	while (str.at(pos) != '\n') {		
		pos++;
	}
	// Do not add the \n to the comment (it will be handled outside)

	printToConsole(str.substr(start, pos - start), FOREGROUND_GREEN); // DEBUG
}


void usingStatement(std::string& str, size_t& pos) {
	// A statement spans until a newline is found (or EOF)

	printToConsole(str.substr(pos, 5), FOREGROUND_BLUE); // DEBUG using
	pos += 5;

	// Skip whitespaces
	while (str.at(pos) == ' ') {
		if (str.at(pos) == ' ') // DEBUG
			printToConsole(" ", FOREGROUND_INTENSITY); // DEBUG
		pos++;
	}

	if (str.substr(pos, 9).compare("namespace") == 0) {
		printToConsole(str.substr(pos, 9), FOREGROUND_BLUE); // DEBUG namespace
		pos += 9;
	}

	// Skip whitespaces
	while (str.at(pos) == ' ') {
		if (str.at(pos) == ' ') // DEBUG
			printToConsole(" ", FOREGROUND_INTENSITY); // DEBUG
		pos++;
	}

	// Whatever identifier we've found until \n
	size_t start = pos; // DEBUG
	while (str.at(pos) != '\n') {			
		pos++;
	}
	printToConsole(str.substr(start, pos-start), FOREGROUND_INTENSITY); // DEBUG
}

void includeStatement(std::string& str, size_t& pos) {
	// A statement spans until a newline is found (or EOF)

	printToConsole(str.substr(pos, 8), FOREGROUND_BLUE); // DEBUG #include
	pos += 8;

	// Skip whitespaces, a quoted string is expected
	while (str.at(pos) == ' ') {
		if (str.at(pos) == ' ') // DEBUG
			printToConsole(" ", FOREGROUND_INTENSITY); // DEBUG
		pos++;
	}

	if (str.at(pos) == '"') {
		size_t debugstart = pos; // DEBUG
		pos++;

		while (str.at(pos) != '"') {
			if (str.at(pos) == '\n')
				return; // Interrupt if a newline is found
			pos++;
		}
		pos++;

		printToConsole(str.substr(debugstart, pos - debugstart), FOREGROUND_RED); // DEBUG
	}

	if (str.at(pos) == '<') {
		size_t debugstart = pos; // DEBUG
		pos++;

		while (str.at(pos) != '>') {
			if (str.at(pos) == '\n')
				return; // Interrupt if a newline is found
			pos++;
		}
		pos++;

		printToConsole(str.substr(debugstart, pos - debugstart), FOREGROUND_RED); // DEBUG
	}
}

void multilineComment(std::string& str, size_t& pos) {
	size_t debugstart = pos; // DEBUG

	pos += 2; // Add the '/*' characters

	// Ignore everything until a */ sequence
	while (str.at(pos) != '*' && str.at(pos+1) != '/')
		pos++;

	// Add '*/'
	pos += 2;

	printToConsole(str.substr(debugstart, pos - debugstart), FOREGROUND_GREEN); // DEBUG

	return; // Return to whatever scope we were in
}

void globalScope(std::string& str, size_t& pos) {

	// We're at global scope, this will end with EOF
	while (true) {

		// Skip newlines and whitespaces
		while (str.at(pos) == ' ' || str.at(pos) == '\r' || str.at(pos) == '\n') {
			if(str.at(pos) == ' ') // DEBUG
				printToConsole(" ", FOREGROUND_INTENSITY); // DEBUG
			if (str.at(pos) == '\n') // DEBUG
				printToConsole("\n", FOREGROUND_INTENSITY); // DEBUG
			pos++;
		}

		if (str.at(pos) == '/' && str.at(pos + 1) == '*') { // Multiline C-style string
			multilineComment(str, pos);
			continue;
		}

		if (str.at(pos) == '/' && str.at(pos + 1) == '/') { // Line comment
			lineCommentStatement(str, pos);
			continue;
		}

		if (str.at(pos) == '#' && str.substr(pos + 1, 7).compare("include") == 0) { // #include
			includeStatement(str, pos);
			continue;
		}

		if (str.substr(pos, 5).compare("using") == 0) { // using
			usingStatement(str, pos);
			continue;
		}

		// TODO simple/unrecognized identifiers (and increment pos!! FGS!)
	}
}

//void continueCurrentScope(std::string& str, size_t& pos) {
//	switch (currentScope) {
//		case GlobalScope: {
//			globalScope(str, pos);
//		}break;
//		default:
//			return; // TODO DROP THIS
//	}
//}


int main(int argc, char** argv)
{
	std::ifstream file;
	file.open("C:\\vectis\\vectis\\TestData\\SimpleFile.cpp", std::ifstream::binary);
	if (!file)
		return -1;
	file.seekg(0, std::ios_base::end);
	size_t size = file.tellg();
	std::string input(size, ' ');
	file.seekg(0);
	file.read(&input[0], size);
	file.close();

	populateKeywords();

	try {
		size_t pos = 0;
		globalScope(input, pos);
	}
	catch (std::exception& ex) {
		std::cout << std::endl << "Parsing terminated!" << std::endl;
	}

	/*std::string input = "+12 -12 -13 90 qwerty";
	std::regex pattern("([+-]?[[:digit:]]+)|([[:alpha:]]+)");

	auto iter_begin = std::sregex_token_iterator(input.begin(), input.end(), pattern, 1);
	auto iter_end = std::sregex_token_iterator();

	for (auto it = iter_begin; it != iter_end; ++it)
	{
		std::ssub_match match = *it;
		std::cout << "Match: " << match.str() << " [" << match.length() << "]" << std::endl;
	}

	std::cout << std::endl << "Done matching..." << std::endl;
	std::string temp;
	std::getline(std::cin, temp);*/

	return 0;
}