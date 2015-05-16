#include <UI/CodeTextEdit/Lexers/CPPLexer.h>

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
  LexerBase(CPPLexerType),
  m_state(CODE)
{
  m_segmentInProgress.reserve (40); // You're coding it wrong if you need more than 40 chars
                                    // per identifier
  populateReservedKeywords (m_reservedKeywords);
}

void CPPLexer::reset() {
  // Reset this lexer's internal state to start another lexing session
  m_state = CODE;
  m_segmentInProgress.clear();
}

void CPPLexer::lexLine(QString &line, std::vector<StyledTextSegment> &ts) {
  qDebug() << "[CPP]lexing for line " << line;

  auto writeOutSegment = [&](int i, Style s) {
    int segmentLen = m_segmentInProgress.size();
    if (segmentLen > 0) { // If we had some characters for a token, now render it a segment

      // Override style if this is a single reserved keyword
      if ( m_reservedKeywords.find( m_segmentInProgress.toStdString() ) != m_reservedKeywords.end() )
        s = Keyword;

      ts.emplace_back(i - segmentLen, segmentLen, s);
      m_segmentInProgress.clear();
    }
  };

  for(int i=0; i<line.size(); ++i) {
    const QChar c = line[i];

    if (m_state == INCLUDE) {
      if (c != '"' && c != '<') {
        m_segmentInProgress += c;
        continue;
      } else {
        writeOutSegment (i, CPP_include);
        m_state = STRING;
        m_stringStartCharacter = c;
        m_segmentInProgress += c;
        continue;
      }
    }

    if (m_state == STRING) {
      if (c != m_stringStartCharacter) {
        m_segmentInProgress += c;
        continue; // No need to check anything else
      } else {
        // End of this string segment reached
        m_state = CODE;
        m_segmentInProgress += c;
        writeOutSegment (i+1, QuotedString);
        continue;
      }
    }

    if(c == '/' && line[i+1] == '/' && m_state != COMMENT && m_state != MULTILINECOMMENT) {
      // Add everything left as a comment segment
      ts.emplace_back(i, line.size()-i, Comment);
      m_state = CODE; // Single line comments don't last more than one line
      m_segmentInProgress.clear();
      return;
    }

    if(c == '"' && m_state == CODE) { // Start a string section
      m_state = STRING;

      // Before starting the new string segment, clear the previous one if there's any
      if (m_segmentInProgress.size() > 0)
        writeOutSegment (i, Normal);

      m_segmentInProgress += c;
      continue;
    }

    if(c == '#' && m_state == CODE &&
       QString(line.data()+i+1).startsWith("include")) { // Start an include
      m_state = INCLUDE;
      writeOutSegment (i, Normal);
      m_segmentInProgress += c;
      continue;
    }
    // TODO: handle macros

    if(isWhitespace(c)) { // Skip whitespaces and signal the end of a token
      writeOutSegment (i, Normal);
      continue;
    } else
      m_segmentInProgress += c;
  }

  writeOutSegment (line.size(), Normal);

  // 1) Keep a style status (e.g. comment? keyword? etc..)
  // 2) Separate words with spaces and ; (terminators)
  // 3) Apply the right style (perhaps also store where a function/class ends/begins.. do it later)
}
