#ifndef UTILS_H
#define UTILS_H

#include <QSyntaxHighlighter>
#include <QString>
#include <memory>

// Highlighters
#include <UI/Highlighters/CPPHighlighter.h>

// Get an instance of a supported and appropriate syntax highlighter for an extension
// or return nullptr if none could be found
inline std::unique_ptr<QSyntaxHighlighter> getSyntaxHighlighterFromExtension(QString extension) {
  if (extension == "cpp" || extension == "c" || extension == "h" ||
      extension == "cxx" || extension == "hpp")
  {
    return std::make_unique<CPPHighlighter>();
  }
  return std::unique_ptr<QSyntaxHighlighter>();
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
