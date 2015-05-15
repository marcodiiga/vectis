#ifndef LEXER_H
#define LEXER_H

#include <QString>
#include <vector>

enum LexerType {
  CPPLexerType
};

struct StyledTextSegment {
  int start;
  int length;
  int style; // TODO: style associated with a lexer
};

// An abstract base class for all the Lexers to implement
class LexerBase {
public:
  LexerBase (LexerType type) : m_type(type) {}
  LexerType getLexerType() const { return m_type; }
  virtual ~LexerBase() = default; // "Thou shalt not cause UB"

  static LexerBase *createLexerOfType(LexerType t);

  virtual void reset() = 0;
  virtual void lexLine(QString& line, std::vector<StyledTextSegment>& ts) = 0;

private:
  LexerType m_type;
};

class isLexer { // Syntax-driven query class
public:
  isLexer(LexerBase *p) : ptr(p) {}
  bool ofType(LexerType t) {
    return ptr->getLexerType() == t;
  }
private:
  LexerBase *ptr;
};

#endif // LEXER_H
