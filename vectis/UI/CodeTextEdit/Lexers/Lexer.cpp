#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <UI/CodeTextEdit/Lexers/CPPLexer.h>

LexerBase* LexerBase::createLexerOfType(LexerType t) {
  switch(t) {
    case CPPLexerType: {
      return new CPPLexer();
    }break;
    default:
      return nullptr;
  }
}
