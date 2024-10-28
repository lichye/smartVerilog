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
        return name + "_" + std::to_string(lindex) + "_" + std::to_string(rindex);
    }
    int getWidth()
    {
        return lindex - rindex + 1;
    }
};

struct SygusVar{
    std::string name;
    SignalType type;
    int width;
};

#endif // UTILS_H