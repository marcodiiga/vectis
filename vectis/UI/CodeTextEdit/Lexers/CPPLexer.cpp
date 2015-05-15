#include <UI/CodeTextEdit/Lexers/CPPLexer.h>

#include <QDebug>

void CPPLexer::reset() {
  // Reset this lexer's internal state to start another lexing session
  //TODO
}

void CPPLexer::lexLine(QString &line, std::vector<StyledTextSegment> &ts) {
  qDebug() << "[CPP]lexing for line " << line;

  // 1) Keep a style status (e.g. comment? keyword? etc..)
  // 2) Separate words with spaces and ; (terminators)
  // 3) Apply the right style (perhaps also store where a function/class ends/begins.. do it later)
}
