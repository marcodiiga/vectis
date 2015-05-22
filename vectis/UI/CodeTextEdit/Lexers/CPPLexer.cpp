#include <UI/CodeTextEdit/Lexers/CPPLexer.h>
#include <regex>
#include <QDebug>

namespace { // Functions reserved for this TU's internal use

  // Detects if a token is a whitespace or newline character
  bool isWhitespace(const QChar c) {
    if (c == ' ' || c == '\n' || c == '\r')
      return true;
    else
      return false;
  }

  void populateReservedKeywords (std::unordered_set<std::string>& set) {
    set.emplace( "alignas" );
    set.emplace( "alignof" );
    set.emplace( "and" );
    set.emplace( "and_eq" );
    set.emplace( "asm" );
    set.emplace( "auto" );
    set.emplace( "bitand" );
    set.emplace( "bitor" );
    set.emplace( "bool" );
    set.emplace( "break" );
    set.emplace( "case" );
    set.emplace( "catch" );
    set.emplace( "char" );
    set.emplace( "char16_t" );
    set.emplace( "char32_t" );
    set.emplace( "class" );
    set.emplace( "compl" );
    set.emplace( "concept" );
    set.emplace( "const" );
    set.emplace( "constexpr" );
    set.emplace( "const_cast" );
    set.emplace( "continue" );
    set.emplace( "decltype" );
    set.emplace( "default" );
    set.emplace( "delete" );
    set.emplace( "do" );
    set.emplace( "double" );
    set.emplace( "dynamic_cast" );
    set.emplace( "else" );
    set.emplace( "enum" );
    set.emplace( "explicit" );
    set.emplace( "export" );
    set.emplace( "extern" );
    set.emplace( "false" );
    set.emplace( "float" );
    set.emplace( "for" );
    set.emplace( "friend" );
    set.emplace( "goto" );
    set.emplace( "if" );
    set.emplace( "inline" );
    set.emplace( "int" );
    set.emplace( "long" );
    set.emplace( "mutable" );
    set.emplace( "namespace" );
    set.emplace( "new" );
    set.emplace( "noexcept" );
    set.emplace( "not" );
    set.emplace( "not_eq" );
    set.emplace( "nullptr" );
    set.emplace( "operator" );
    set.emplace( "or" );
    set.emplace( "or_eq" );
    set.emplace( "private" );
    set.emplace( "protected" );
    set.emplace( "public" );
    set.emplace( "register" );
    set.emplace( "reinterpret_cast" );
    set.emplace( "requires" );
    set.emplace( "return" );
    set.emplace( "short" );
    set.emplace( "signed" );
    set.emplace( "sizeof" );
    set.emplace( "static" );
    set.emplace( "static_assert" );
    set.emplace( "static_cast" );
    set.emplace( "struct" );
    set.emplace( "switch" );
    set.emplace( "template" );
    set.emplace( "this" );
    set.emplace( "thread_local" );
    set.emplace( "throw" );
    set.emplace( "true" );
    set.emplace( "try" );
    set.emplace( "typedef" );
    set.emplace( "typeid" );
    set.emplace( "typename" );
    set.emplace( "union" );
    set.emplace( "unsigned" );
    set.emplace( "using" );
    set.emplace( "virtual" );
    set.emplace( "void" );
    set.emplace( "volatile" );
    set.emplace( "wchar_t" );
    set.emplace( "while" );
    set.emplace( "xor" );
    set.emplace( "xor_eq" );
    set.emplace( "nullptr" );
  }



}

CPPLexer::CPPLexer() :
  LexerBase(CPPLexerType)
{
  populateReservedKeywords (m_reservedKeywords);
  m_classKeywordActiveOnScope = -2;
}

void CPPLexer::reset() {
  // Reset this lexer's internal state to start another lexing session
  m_classKeywordActiveOnScope = -2;
  std::stack<int> empty;
  std::swap( m_scopesStack, empty ); // Dumb clearing mechanism
  m_adaptPreviousSegments.clear();
}

void CPPLexer::lexInput(std::string& input, StyleDatabase& sdb) {

  str = &input;
  sdb.styleSegment.clear(); // Relex everything // TODO - lex from a position forward?
  styleDb = &sdb;
  pos = 0;

  try {
    globalScope();
  }
  catch (std::out_of_range&) {
    qDebug() << "Parsing terminated!";
  }
}


//==---------------------------------------------------------------------------==//
//                         Scopes handling functions                             //
//==---------------------------------------------------------------------------==//


// Utility function: adds a segment to the style database
void CPPLexer::addSegment(size_t pos, size_t len, Style style) {
  styleDb->styleSegment.emplace_back(pos, len, style);
}

void CPPLexer::classDeclarationOrDefinition() {
  // TODO: fw decl or def
}

void CPPLexer::declarationOrDefinition() { // A scope declaration or definition
  // of a function, class (or some macro-ed stuff e.g. CALLME();) or local variables

  // Skip whitespaces
  while (str->at(pos) == ' ') {
    pos++;
  }

 const char *ptr = str->c_str() + pos; // debug

  // Handle any keyword or identifier until a terminator character
  bool foundSegment = false;
  size_t startSegment = pos;
  while (    (str->at(pos) >= '0' && str->at(pos) <= '9')
          || (str->at(pos) >= 'A' && str->at(pos) <= 'Z')
          || (str->at(pos) >= 'a' && str->at(pos) <= 'z')
          ||  str->at(pos) == '_') {
    pos++;
  }
  if (pos > startSegment) { // We found something
    Style s = Normal;

    // It might be a reserved keyword
    std::string segment = str->substr(startSegment, pos - startSegment);
    if (m_reservedKeywords.find(segment) != m_reservedKeywords.end()) {
      // For purely aesthetic reasons, style the keywords which aren't private/protected/public
      // in an inner scope with a different style
      if (m_scopesStack.size() > 0 && segment.compare("protected") != 0
          && segment.compare("private") != 0 && segment.compare("public") != 0)
        s = KeywordInnerScope;
      else
        s = Keyword;
      if ( (segment.compare("class") == 0 || segment.compare("struct") == 0) &&
           m_classKeywordActiveOnScope == -2 /* No inner class support for now */)
        m_classKeywordActiveOnScope = -1; // We keep track of this since a class scope is *not* a local
        // scope, but rather should be treated as the global scope. If we encounter a ';' before any '{', this
        // value gets back to -2, i.e. 'no class keyword active'. If we join a scope, this gets set to the
        // scope number and from that point forward whenever we're in that scope, no function call can be
        // used (only declarations). If we pop out of that function scope, it returns to -2.
    } else {

      // Or perhaps a literal (e.g. 11)
      std::regex lit("\\d+[uUlL]?[ull]?[ULL]?[UL]?[ul]?[ll]?[LL]?");
      std::regex lit2("0[xbX][\\da-fA-F]+");
      if(std::regex_match(segment, lit) || std::regex_match(segment, lit2))
        s = Literal;

    }

    // Assign a Keyword or Normal style and later, if we find (, make it a function declaration
    addSegment(startSegment, pos - startSegment, s);
    foundSegment = true;
  }
  // Skip whitespaces and stuff that we're not interested in
  while (str->at(pos) == ' ' || str->at(pos) == '\n') {
    pos++;
  }

  if (str->at(pos) == '(') {

    // Check for the scopes stack and, if we're not in a global scope, mark this as function call.
    // Notice that class member functions aren't marked as function calls but rather as identifiers.
    if (foundSegment && !m_scopesStack.empty() && m_classKeywordActiveOnScope != m_scopesStack.top() &&
        styleDb->styleSegment[styleDb->styleSegment.size()-1].style != Keyword) {

      styleDb->styleSegment[styleDb->styleSegment.size()-1].style = FunctionCall;

      // Also set the same style for all the linked previous segments
      for(auto i : m_adaptPreviousSegments)
        styleDb->styleSegment[i].style = FunctionCall;
      m_adaptPreviousSegments.clear();

    } else if (foundSegment && (m_scopesStack.empty() || m_classKeywordActiveOnScope == m_scopesStack.top() ) &&
             styleDb->styleSegment[styleDb->styleSegment.size()-1].style != Keyword) {

      styleDb->styleSegment[styleDb->styleSegment.size()-1].style = Identifier;

      // Also set the same style for all the linked previous segments
      for(auto i : m_adaptPreviousSegments)
        styleDb->styleSegment[i].style = Identifier;
      m_adaptPreviousSegments.clear();
    }

    pos++; // Eat the '('
  }

  if (str->at(pos) == ':' && str->at(pos+1) == ':') { // :: makes the previous segment part of the new one
    if (styleDb->styleSegment.size() > 0)
      m_adaptPreviousSegments.push_back(styleDb->styleSegment.size()-1);
  }

  const char *ptr2 = str->c_str() + pos; // debug

  if (foundSegment == false) { // We couldn't find a normal identifier
    if (str->at(pos) == '{') { // Handle entering/exiting scopes
      pos++;
      m_scopesStack.push(m_scopesStack.size());

      if (m_classKeywordActiveOnScope == -1)
        m_classKeywordActiveOnScope = m_scopesStack.top(); // Joined a class scope

    } else if (str->at(pos) == '}') {
      pos++;

      if (m_classKeywordActiveOnScope == m_scopesStack.top())
        m_classKeywordActiveOnScope = -2; // Exited a class scope

      m_scopesStack.pop();
    } else if (str->at(pos) == '"' || str->at(pos) == '\'') {

      // A quoted string
      char startCharacter = str->at(pos);
      startSegment = pos++;
      while(str->at(pos) != startCharacter)
        ++pos;
      pos++; // Include the terminal character

      addSegment(startSegment, pos - startSegment, QuotedString);
    } else {

      // We really can't identify this token, just skip it and assign a regular style

      if (str->at(pos) == ';' && m_classKeywordActiveOnScope == -1)
        m_classKeywordActiveOnScope = -2; // Deactivate the class scope override

      pos++;
    }
  }







  //    // Handle any other keyword or identifier (this could be a function name)
  //    bool foundSegment = false;
  //    size_t startSegment = pos;
  //    while (str->at(pos) != ' ' && str->at(pos) != '(' && str->at(pos) != ';' && str->at(pos) != '{') {
  //      pos++;
  //    }
  //    if (pos > startSegment) { // We found something else
  //      // This might be a declaration if we find another pair of parenthesis or a variable name.
  //      // Assign a Normal style and later, if we find (, make it a function declaration
  //      addSegment(startSegment, pos - startSegment, Normal);
  //      foundSegment = true;
  //    }
  //    // Skip whitespaces and stuff that we're not interested in
  //    while (str->at(pos) == ' ' || str->at(pos) == '\n') {
  //      pos++;
  //    }

  //    if(str->at(pos) == ';') {
  //      pos++;
  //      break; // Hit a terminator
  //    }




//  // First check if this is a class forward declaration or definition
//  if (str->substr(pos, 5).compare("class") == 0) { // class
//    classDeclarationOrDefinition();
//    return;
//  }



//  do { // Continuously parse identifiers/arguments until we hit a terminator

//    // Handle any other keyword or identifier (this could be a function name)
//    bool foundSegment = false;
//    size_t startSegment = pos;
//    while (str->at(pos) != ' ' && str->at(pos) != '(' && str->at(pos) != ';' && str->at(pos) != '{') {
//      pos++;
//    }
//    if (pos > startSegment) { // We found something else
//      // This might be a declaration if we find another pair of parenthesis or a variable name.
//      // Assign a Normal style and later, if we find (, make it a function declaration
//      addSegment(startSegment, pos - startSegment, Normal);
//      foundSegment = true;
//    }
//    // Skip whitespaces and stuff that we're not interested in
//    while (str->at(pos) == ' ' || str->at(pos) == '\n') {
//      pos++;
//    }

//    if(str->at(pos) == ';') {
//      pos++;
//      break; // Hit a terminator
//    }

//    const char *ptr = str->c_str() + pos; // debug

//    // Handle any (..) section
//    if (str->at(pos) == '(') {
//      // Global scope function call

//      // This also means the previous segment was a declaration or a function call (if we're inside a function)
//      if (foundSegment && m_scopesStack.empty())
//        styleDb->styleSegment[styleDb->styleSegment.size()-1].style = Identifier;
//      else if (foundSegment) // We're in an inner scope
//        styleDb->styleSegment[styleDb->styleSegment.size()-1].style = FunctionCall;

//      while (str->at(pos) != ')' && str->at(pos) != ';') {

//        if (str->at(pos) == '{') {
//          pos++;
//          m_scopesStack.push(m_scopesStack.size());
//        }

//        if (str->at(pos) == '}') {
//          pos++;
//          m_scopesStack.pop();
//          if (m_scopesStack.empty())
//            break;
//        }

//        pos++;
//      }
//      pos++; // Eat the )
//    }
//    // Skip whitespaces and stuff that we're not interested in
//    while (str->at(pos) == ' ' || str->at(pos) == '\n') {
//      pos++;
//    }

//    // There might be a function body at this point, if there is: handle it
//    if (str->at(pos) == '{') {
//      pos++;
//      m_scopesStack.push(m_scopesStack.size());
//    }

//    if (str->at(pos) == '}') {
//      pos++;
//      m_scopesStack.pop();
//      if (m_scopesStack.empty())
//        break;
//    }

//  } while (true);

}

void CPPLexer::defineStatement() {
  // A define statement is a particular one: it might span one or more lines

  addSegment(pos, 7, Keyword); // #define
  pos += 7;

  // Skip whitespaces
  while (str->at(pos) == ' ') {
    pos++;
  }

  // Now we might have something like
  // #define MYMACRO XX
  // or
  // #define MYMACRO(a,b,c..) something
  // or even
  // #define MYMACRO XX \
  //                 multiline
  //

  size_t startSegment = pos;
  while (str->at(pos) != '(' && str->at(pos) != ' ') {
    pos++;
  }
  addSegment(startSegment, pos - startSegment, Identifier);

  // Regular style for all the rest. A macro, even multiline, ends when a newline not preceded
  // by \ is found
  startSegment = pos;
  while (!(str->at(pos) != '\\' && str->at(pos+1) == '\n')) {
    pos++;
  }
  addSegment(startSegment, pos - startSegment, Normal);
  pos++; // Eat the last character

  // Do not add the \n to the comment (it will be handled outside)
}

void CPPLexer::lineCommentStatement() {
  // A statement spans until a newline is found (or EOF)

  size_t startSegment = pos;

  // Skip everything until \n
  while (str->at(pos) != '\n') {
    pos++;
  }
  // Do not add the \n to the comment (it will be handled outside)

  addSegment(startSegment, pos - startSegment, Comment);
}


void CPPLexer::usingStatement() {
  // A statement spans until a newline is found (or EOF)

  addSegment(pos, 5, Keyword); // using
  pos += 5;

  // Skip whitespaces
  while (str->at(pos) == ' ') {
    pos++;
  }

  if (str->substr(pos, 9).compare("namespace") == 0) {
    addSegment(pos, 9, Keyword); // namespace
    pos += 9;
  }

  // Skip whitespaces
  while (str->at(pos) == ' ') {
    pos++;
  }

  // Whatever identifier we've found until \n
  size_t startSegment = pos;
  while (str->at(pos) != '\n') {
    pos++;
  }
  addSegment(startSegment, pos - startSegment, Normal);
}

void CPPLexer::includeStatement() {
  // A statement spans until a newline is found (or EOF)

  addSegment(pos, 8, Keyword); // #include
  pos += 8;

  // Skip whitespaces, a quoted string is expected
  while (str->at(pos) == ' ') {
    pos++;
  }

  if (str->at(pos) == '"') {
    size_t segmentStart = pos;
    pos++;

    while (str->at(pos) != '"') {
      if (str->at(pos) == '\n')
        return; // Interrupt if a newline is found
      pos++;
    }
    pos++;

    addSegment(segmentStart, pos - segmentStart, QuotedString);
  }

  if (str->at(pos) == '<') {
    size_t segmentStart = pos;
    pos++;

    while (str->at(pos) != '>') {
      if (str->at(pos) == '\n')
        return; // Interrupt if a newline is found
      pos++;
    }
    pos++;

    addSegment(segmentStart, pos - segmentStart, QuotedString);
  }
}

void CPPLexer::multilineComment() {
  size_t segmentStart = pos;

  pos += 2; // Add the '/*' characters

  // Ignore everything until a */ sequence
  while (str->at(pos) != '*' && str->at(pos+1) != '/')
    pos++;

  // Add '*/'
  pos += 2;

  addSegment(segmentStart, pos - segmentStart, Comment);

  return; // Return to whatever scope we were in
}

void CPPLexer::globalScope() {

  // We're at global scope, this will end with EOF
  while (true) {

     const char *ptr = str->c_str() + pos; // debug

    // Skip newlines and whitespaces
    while (str->at(pos) == ' ' || str->at(pos) == '\r' || str->at(pos) == '\n') {
      // addSegment(pos, 1, Normal); // This is not needed
      pos++;
    }

    if (str->at(pos) == '/' && str->at(pos + 1) == '*') { // Multiline C-style string
      multilineComment();
      continue;
    }

    if (str->at(pos) == '/' && str->at(pos + 1) == '/') { // Line comment
      lineCommentStatement();
      continue;
    }

    if (str->at(pos) == '#' && str->substr(pos + 1, 7).compare("include") == 0) { // #include
      includeStatement();
      continue;
    }

    if (str->at(pos) == '#' && str->substr(pos + 1, 6).compare("define") == 0) { // #define
      defineStatement();
      continue;
    }

    if (str->substr(pos, 5).compare("using") == 0) { // using
      usingStatement();
      continue;
    }

    declarationOrDefinition(); // Last chance: something custom

    // TODO simple/unrecognized identifiers (and increment pos!! FGS!)
    //pos++;
  }
}
