#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>

#define smartVerbose 0
#define runRandomState true
// Verbose level 0: No debug information
// Verbose level 1: module calls - level
// Verbose level 2: function calls - level
// Verbose level 3: function details and data flows - level

inline void print(std::string message)
{
   std::cout<<message<<std::endl;
}

inline void printDebug(std::string message, int level)
{
    if (smartVerbose >= level)
    {
        std::cout << message << std::endl;
    }
}

inline void printError(std::string message)
{
    std::cerr << message << std::endl;
}

enum SignalType{
    BOOLEAN = 0,
    BITS = 1,
    DOUBLE = 2 ,
    UNKNOWNSINGALTYPES = 3
};

enum BitType{
    ZERO,
    ONE,
    X,
    Z
};

//matain the signal information, to convert all other signal types to this format
struct Signal{
    std::string name;
    std::string moduleName;
    SignalType type;
    int lindex;
    int rindex;
    bool operator<(const Signal& rhs) const
    {   
        return name != rhs.name ? name < rhs.name :
               moduleName != rhs.moduleName ? moduleName < rhs.moduleName :
               type != rhs.type ? type < rhs.type :
               lindex != rhs.lindex ? lindex < rhs.lindex :
               rindex < rhs.rindex;
    }
    std::string toSygusName()
    {   
        //TODO:
        //this is for debug purpose
        if(true)
            return name;
            
        if(type == SignalType::BOOLEAN)
            return name + "_" + cleanName();
        else
            return name + "_" + std::to_string(lindex) + "_" + std::to_string(rindex) + "_" + cleanName();
    }
    int getWidth()
    {
        return lindex - rindex + 1;
    }
    std::string cleanName()
    {
        std::string result="";
        for (char& ch : moduleName) {
            if (std::isalnum(static_cast<unsigned char>(ch))) {
                result+=ch;
            }
        }
        return result;
    }
};

struct SygusVar{
    std::string name;
    SignalType type;
    int width;
};

#endif // UTILS_H