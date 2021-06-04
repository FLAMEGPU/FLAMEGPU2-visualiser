#ifndef SRC_UTIL_VISEXCEPTION_H_
#define SRC_UTIL_VISEXCEPTION_H_

#include <string>
#include <exception>
#include <cstdarg>

namespace flamegpu {
namespace visualiser {

/**
 * If this macro is used instead of 'throw', VisException will 
 * prepend '__FILE__ (__LINE__): ' to err_message 
 */
#define THROW ::flamegpu::visualiser::VisException::setLocation(__FILE__, __LINE__); throw

/*! Base class for exceptions thrown */
class VisException : public std::exception {
 public:
    /**
     * A constructor
     * @brief Constructs the VisException object
     * @note Attempts to append '__FILE__ (__LINE__): ' to err_message
     */
     VisException();
    /**
     * @brief Returns the explanatory string
     * @return Pointer to a nullptr-terminated string with explanatory information. The pointer is guaranteed to be valid at least until the exception object from which it is obtained is destroyed, or until a non-const member function on the VisException object is called.
     */
     const char *what() const noexcept override;

    /**
     * Sets internal members file and line, which are used by constructor
     */
     static void setLocation(const char *_file, const unsigned int &_line);

 protected:
    /**
     * Parses va_list to a string using vsnprintf
     */
     static std::string parseArgs(const char * format, va_list argp);
    std::string err_message;

 private:
    static const char *file;
    static unsigned int line;
};

/**
 * Macro for generating common class body for derived classes of VisException
 */
#define DERIVED_VisException(name, default_msg)\
class name : public VisException {\
 public:\
    explicit name(const char *format = default_msg, ...) {\
        va_list argp;\
        va_start(argp, format);\
        err_message += parseArgs(format, argp);\
        va_end(argp);\
    }\
}


/////////////////////
// Derived Classes //
/////////////////////

/**
 * Defines a type of object to be thrown as exception.
 * It reports errors that are due to unintended states
 * These mostly replace assert() statements
 */
DERIVED_VisException(VisAssert, "Visualisation entered an unexpected state!");

/**
 * Defines a type of object to be thrown as exception.
 * It reports errors that are due to OpenGL errors
 */
DERIVED_VisException(GLError, "OpenGL returned an error code!");
/**
 * Defines a type of object to be thrown as exception.
 * It reports errors that are due to Resource loading
 */
DERIVED_VisException(ResourceError, "Resource was not found!");

/**
 * Defines a type of object to be thrown as exception.
 * It reports errors that are due to Font loading
 */
DERIVED_VisException(FontLoadingError, "Error during font loading!");
/**
 * Defines a type of object to be thrown as exception.
 * It reports errors that are due to bad data being passed to Draw class
 */
DERIVED_VisException(SketchError, "Error during font loading!");
/**
 * Defines an error reported when the expect input/output file path does not exist
 */
DERIVED_VisException(InvalidFilePath, "File does not exist.");
/**
 * Errors that occur whilst using FreeType
 */
DERIVED_VisException(FreeTypeError, "Freetype returned an error.");
/**
 * Something bad inside an entity
 */
DERIVED_VisException(EntityError, "Something went wrong.");


/**
 * Lazy replacement for assert()
 */
#define visassert(cdn) if (!(cdn)) { THROW VisAssert("VisAssert: '#cdn' failed!\n");}

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_UTIL_VISEXCEPTION_H_
