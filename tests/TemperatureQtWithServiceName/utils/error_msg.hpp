#ifndef ERROR_MSG_H_
#define ERROR_MSG_H_

#include <string>
#include <sstream>
#include <utility>
#include <cstddef>

namespace utils
{

template<typename... Args>
int print(std::ostream& s, Args&... args)
{
    using super = int[];
    return super{ 0, ((s << std::forward<Args>(args)), 0)...}[0];
}

template<typename... Args>
std::string buildStringFromParts(Args const&... args)
{
    std::stringstream msg;
    print(msg, args...);
    return msg.str();
}

template<typename... Args>
std::string buildErrorMessage(Args const&... args)
{
    return buildStringFromParts(args...);
}

}

#endif /* ERROR_MSG_H_ */