#ifndef CPPLEXER_H
#define CPPLEXER_H

#include <UI/CodeTextEdit/Lexers/Lexer.h>

class CPPLexer : public LexerBase {
public:
  CPPLexer() : LexerBase(CPPLexerType) {}

  void reset();
  void lexLine(QString& line, std::vector<StyledTextSegment>& ts);
};

#endif // CPPLEXER_H
