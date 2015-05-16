#ifndef LEXER_H
#define LEXER_H

#include <QString>
#include <vector>

// A list of supported lexers (this might be expanded in the future)
enum LexerType {
  CPPLexerType
};

// A list of styles for the segments found by the lexer
enum Style {

  //==-- Generic styles --==//
  Normal,
  Keyword,
  Comment,
  QuotedString,

  //==-- C++ specific styles --==//
  CPP_include
};

struct StyledTextSegment {
  StyledTextSegment(int s, int l, Style st) : start(s), length(l), style(st) {}
  int start;
  int length;
  Style style = Normal; // Style associated with this segment
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
