#include <UI/CodeTextEdit/Lexers/Lexer.h>
#include <UI/CodeTextEdit/Lexers/CPPLexer.h>

SyntaxHighlight getSuggestedSyntaxHighlightFromExtension(QString extension) {
  static std::unordered_map<std::string, SyntaxHighlight> extensionToSyntaxHighlighting = {
    {"txt", NONE},

    {"cpp", CPP},
    {"h", CPP},
    {"hpp", CPP},
    {"cxx", CPP},
    {"c", CPP}
  };

  auto it = extensionToSyntaxHighlighting.find(extension.toStdString());
  if (it == extensionToSyntaxHighlighting.end())
    return NONE;
  else
    return it->second;
}

LexerBase* LexerBase::createLexerOfType(LexerType t) {
  switch(t) {
    case CPPLexerType: {
      return new CPPLexer();
    }break;
    default:
      return nullptr;
  }
}
