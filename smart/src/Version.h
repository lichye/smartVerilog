// The one place the version number lives. Bump it here; --version,
// --check-env, the generated header comment and the run log all read it.
#ifndef SMART_VERSION_H
#define SMART_VERSION_H

namespace smart {
inline const char* version() { return "1.0.0"; }
}  // namespace smart

#endif  // SMART_VERSION_H
