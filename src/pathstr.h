#ifndef PATHSTR_H_
#define PATHSTR_H_

#include <iosfwd>
#include <string>

#ifdef _WIN32
  using PathChar = wchar_t;
  #define ALOGSTR L
#else
using PathChar = char;
  #define ALOGSTR
#endif

using PathStr = std::basic_string<PathChar>;
using PathStrIStream = std::basic_istringstream<PathChar>;

#endif /* PATHSTR_H_ */
