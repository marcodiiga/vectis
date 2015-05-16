#ifndef CPPLEXER_H
#define CPPLEXER_H

#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <unordered_set>
#include <string>

class CPPLexer : public LexerBase {
public:
  CPPLexer();

  void reset();
  void lexLine(QString& line, std::vector<StyledTextSegment>& ts);

private:
  // States the lexer can find itself into
  enum LexerStates {CODE, STRING, COMMENT, MULTILINECOMMENT, INCLUDE};
  LexerStates m_state;
  std::unordered_set<std::string> m_reservedKeywords;
  QString m_segmentInProgress; // A cache variable to keep in-progress segments
  QChar m_stringStartCharacter; // An include string can be started by " or <
};

#endif // CPPLEXER_H
