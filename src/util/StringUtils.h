#ifndef SRC_UTIL_STRINGUTILS_H_
#define SRC_UTIL_STRINGUTILS_H_
#include <string>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <cctype>
#include <memory>

namespace su {
namespace internal {
#ifdef _MSC_VER
struct MatchPathSeparator {
    bool operator()(char ch) const {
        return ch == '\\' || ch == '/';
    }
};
#else
struct MatchPathSeparator {
    bool operator()(char ch) const {
        return ch == '/';
    }
};
#endif
inline char toLower_char(char c) {
    return static_cast<char>(std::tolower(c));
}
}  // namespace internal
inline std::string toLower(const std::string &in) {
    std::string out(in.size(), '\0');  // Create empty string of null terminating
    std::transform(in.begin(), in.end(), out.begin(), su::internal::toLower_char);
    return out;
}
inline bool beginsWith(const std::string &str, const std::string &begin, bool caseSensitive = true) {
    if (caseSensitive) {
        for (unsigned int i = 0; i < str.size() && i < begin.size(); ++i) {
            if (str[i] != begin[i])
                return false;
        }
        return str.size() >= begin.size();
    }
    return beginsWith(toLower(str), toLower(begin), true);
}
inline bool endsWith(const std::string &str, const std::string &end, bool caseSensitive = true) {
    if (caseSensitive) {
        for (unsigned int i = 1; i <= str.size() && i <= end.size(); ++i) {
            if (str[str.size() - i] != end[end.size() - i])
                return false;
        }
        return str.size() >= end.size();
    }
    return endsWith(toLower(str), toLower(end), true);
}
/**
 * Returns the filename from the provided file path
 * @param filePath A null terminated string holding a file path
 * @return The filename extracted from the string
 * @note This has operating system dependent behaviour, Linux consider \\ a valid path
 */
inline std::string getFilenameFromPath(const std::string &filePath) {
    std::string pathname(filePath);
    std::string result = std::string(
        std::find_if(pathname.rbegin(), pathname.rend(), internal::MatchPathSeparator()).base(),
        pathname.end());
    return result;
}
/**
 * Returns the folder path from the provided file path
 * @param filePath A null terminated string holding a file path
 * @return The folder path extracted from the string
 * @note This has operating system dependent behaviour, Linux consider \\ a valid path
 */
inline std::string getFolderFromPath(const std::string &filePath) {
    std::string pathname(filePath);
    std::string result = std::string(
        pathname.begin(),
        std::find_if(pathname.rbegin(), pathname.rend(), internal::MatchPathSeparator()).base());
    return result;
}
/**
 * Returns the filename sans extension from the provided file path
 * @param filename A null terminated string holding a file name
 * @return The filename sans extension
 */
inline std::string removeFileExt(const std::string &filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}

// https:// stackoverflow.com/a/26221725/1646387
#pragma warning(disable : 4996)
template<typename ... Args>
inline std::string format(const std::string& format, Args ... args) {
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;  //  Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1);  //  We don't want the '\0' inside
#ifdef _MSC_VER
#undef  snprintf
#endif
}
inline bool contains(const std::string& haystack, const std::string& needle, bool caseSensitive = true) {
    if (caseSensitive)
        return haystack.find(needle) != std::string::npos;
    return contains(toLower(haystack), toLower(needle), true);
}  // namespace
}  // namespace su

#endif  // SRC_UTIL_STRINGUTILS_H_
