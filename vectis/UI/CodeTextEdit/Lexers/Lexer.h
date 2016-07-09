#ifndef LEXER_H
#define LEXER_H

#include <QString>
#include <vector>
#include <unordered_map>
#include <string>

// A list of supported lexers (this might be expanded in the future)
enum LexerType {
  CPPLexerType
};

// A lexer might support multiple kind of syntax highlights
enum SyntaxHighlight { NONE, CPP };

// This function helps getting the right Syntax Highlighting from a common file extension.
// Returns NONE if no syntax highlight scheme could be associated to this extension.
SyntaxHighlight getSuggestedSyntaxHighlightFromExtension(QString extension);

// A list of styles for the segments found by the lexer
enum Style {

  //==-- Generic styles --==//
  Normal,
  Keyword,
  KeywordInnerScope,
  Comment,
  QuotedString,
  Identifier, // E.g. a function name (fully qualified or unqualified) or a macro
  FunctionCall,
  Literal,

  //==-- C++ specific styles --==//
  CPP_include
};

struct StyleDatabase {
  struct StyleSegment {
    StyleSegment(size_t s, size_t c, Style st) : start(s), count(c), style(st) {}
    size_t start;
    size_t count;
    Style style;
  };

  // Stores the beginning and count of characters per each styled segment
  std::vector<StyleSegment> styleSegment;
};

// An abstract base class for all the Lexers to implement
class LexerBase {
public:
  LexerBase (LexerType type) : m_type(type) {}
  LexerType getLexerType() const { return m_type; }
  virtual ~LexerBase() = default; // "Thou shalt not cause UB"

  static LexerBase *createLexerOfType(LexerType t);

  virtual void reset() = 0;
  virtual void lexInput(std::string input, StyleDatabase& sdb) = 0;

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
