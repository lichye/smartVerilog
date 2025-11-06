#ifndef SETUPS_H
#define SETUPS_H

// Property mining Settings
#define unboundChecker false
#define LTLproperty true
#define LTLpropertyDepth 1

// Debugging Settings
#define smartVerbose 1
#define deleteTempFile true
#define runRandomState true

inline void print(std::string message)
{
   std::cout<<message<<std::endl;
}

inline void printDebug(std::string message, int level)
{
    if (smartVerbose >= level)
    {   
        for(int i=0;i<level;i++){
            std::cout<<"\t";
        }
        std::cout << message << std::endl;
    }
}

inline void printError(std::string message)
{
    std::cerr << message << std::endl;
}

#endif // SETUPS_H