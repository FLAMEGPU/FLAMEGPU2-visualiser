#ifndef SRC_UTIL_WARNINGS_H_
#define SRC_UTIL_WARNINGS_H_

/**
 * Series of macros for disabling warnings certain warnings across different compilers.
 */

// define macros for linux compilers which use the same pragmas
#if defined(__GNUC__) || defined(__clang__)
#define WRAPPED_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNING_PUSH WRAPPED_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP WRAPPED_PRAGMA(GCC diagnostic pop)
#define DISABLE_WARNING(name) WRAPPED_PRAGMA(GCC diagnostic ignored #name)

// Explicit warnings that are disabled (on one or more platforms)
#define DISABLE_WARNING_DEPRECATED

// Pragmas for msvc
#elif defined(_MSC_VER)

// MSVC also sets the warning level to 3?
#define DISABLE_WARNING_PUSH __pragma(warning(push, 3))
#define DISABLE_WARNING_POP __pragma(warning(pop))
#define DISABLE_WARNING(num) __pragma(warning(disable : num))

// Explicit warnings that are disabled (on one or more platforms)
#define DISABLE_WARNING_DEPRECATED DISABLE_WARNING(4996)

// Other compilers are not supported, but add empty defines just in case
#else

#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP

#define DISABLE_WARNING_DEPRECATED

#endif



#endif  // SRC_UTIL_WARNINGS_H_
