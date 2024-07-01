#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests
{
    static std::string Registration = "Reg";
    static std::string Hello = "Hel";
    static std::string Trading = "Tra";
    static std::string Status = "Sta";
    static std::string Free = "Free";
}

#endif //CLIENSERVERECN_COMMON_HPP
