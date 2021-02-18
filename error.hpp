#ifndef NEXTLIB_ERROR_HPP
#define NEXTLIB_ERROR_HPP

#include <stdexcept>

namespace Next
{
    class Exception : std::runtime_error {
    public:
        Exception(const char* message) throw() : std::runtime_error(message) {
            what_message = message;
        }

        const char* what() const throw() {
            return what_message.c_str();
        }
    private:
        std::string what_message;
    };
}

#endif // NEXTLIB_ERROR_HPP