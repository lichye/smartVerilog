#ifndef UTILS_H
#define UTILS_H
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
};
#endif // UTILS_H