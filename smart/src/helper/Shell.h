#ifndef SMART_HELPER_SHELL_H
#define SMART_HELPER_SHELL_H

#include <string>

namespace smart {
namespace helper {

// Quote one argument for /bin/sh. In particular, a literal apostrophe has to
// close the single-quoted string, be emitted in double quotes, and reopen it.
inline std::string shellQuote(const std::string& text) {
    std::string quoted = "'";
    for (const char c : text) {
        if (c == '\'')
            quoted += "'\"'\"'";
        else
            quoted += c;
    }
    return quoted + "'";
}

}  // namespace helper
}  // namespace smart

#endif  // SMART_HELPER_SHELL_H
