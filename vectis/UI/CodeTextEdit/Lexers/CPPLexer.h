#ifndef CPPLEXER_H
#define CPPLEXER_H

#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <unordered_set>
#include <string>

class CPPLexer : public LexerBase {
public:
  CPPLexer();

  void reset() override;
  void lexInput(std::string& input, StyleDatabase& sdb) override;

private:
  //// States the lexer can find itself into
  //enum LexerStates {CODE, STRING, COMMENT, MULTILINECOMMENT, INCLUDE};
  //LexerStates m_state;
  std::unordered_set<std::string> m_reservedKeywords;

  // The contents of the document and the position we're lexing at
  std::string *str;
  size_t pos;
  StyleDatabase *styleDb;

  void addSegment(size_t pos, size_t len, Style style);

  void lineCommentStatement();
  void usingStatement();
  void includeStatement();
  void multilineComment();
  void globalScope();

};

#endif // CPPLEXER_H
