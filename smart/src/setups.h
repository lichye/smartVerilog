#ifndef SETUPS_H
#define SETUPS_H

#define smartVerbose 3
#define deleteTempFile false
#define runRandomState true
#define unboundChecker false
#define LTLproperty true
#define LTLpropertyDepth 1


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

#endif // SETUPS_H