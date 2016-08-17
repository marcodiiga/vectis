#ifndef UTILS_H
#define UTILS_H

#include <QSyntaxHighlighter>
#include <QString>
#include <memory>

// Highlighters
#include <UI/Highlighters/CPPHighlighter.h>
#include <UI/Highlighters/WhiteTextHighlighter.h>

// Get an instance of a supported and appropriate syntax highlighter for an extension
// or return nullptr if none could be found. Memory cleanup is user's task.
inline QSyntaxHighlighter* getSyntaxHighlighterFromExtension(QString extension) {
  if (extension == "cpp" || extension == "c" || extension == "h" ||
      extension == "cxx" || extension == "hpp")
  {
    return new CPPHighlighter();
  }  
  return new WhiteTextHighlighter();
}

template <typename T>
inline T clamp(T val, T min, T max) {
  if (val < min)
    return min;
  else if (val > max)
    return max;
  else
    return val;
}

#endif // UTILS_H
