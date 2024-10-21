#if !defined(VALUE_H)
#define VALUE_H

#include "VCDValue.hpp"
enum SignalType{
    BOOLEAN,
    BITS,
    DOUBLE,
    UNKNOWNSINGALTYPES
};

enum BitType{
    ZERO,
    ONE,
    X,
    Z
};


class Value{
    public:
        Value(VCDValue* value);
        Value();
        ~Value();
        std::string toString();
    private:
        SignalType type;
        union valstore{
            BitType bitValue;
            std::vector<BitType> *bitVector;
            double realValue;
        }value;
        char Bit2Char(BitType bit);
};
#endif // VALUE_H

