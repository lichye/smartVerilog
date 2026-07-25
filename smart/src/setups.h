#ifndef SETUPS_H
#define SETUPS_H

// Debugging Settings
//
// Verbosity is a RUNTIME setting now (`smart -v` / `-q`); it used to be the
// compile-time constant `smartVerbose`, which meant the only way to quieten a
// run was to rebuild. 0 keeps the tool quiet, 1 is the old default and what a
// block writes into its own log.
inline int smartVerbosity = 0;
#define deleteTempFile true
#define runRandomState true

inline void print(std::string message)
{
   std::cout<<message<<std::endl;
}

inline void printDebug(std::string message, int level)
{
    if (smartVerbosity >= level)
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