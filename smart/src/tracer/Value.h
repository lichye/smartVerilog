#if !defined(VALUE_H)
#define VALUE_H

#include "VCDValue.hpp"
#include "utils.h"
#include <random>

class Value{
    public:
        static Value* makeXValue(SignalType,int);
        static Value* makeRandomValue(SignalType,int);
        static Value* makeNewValue(Value*,SignalType,int);
        static void setSeed(int);

        Value(VCDValue* value);
        Value();
        ~Value();
        Value* clone();
        std::string toString();
        std::string toSyGusString();
        std::string toVerilogString();
        bool isUndefined();
        
        
    private:
        static std::random_device rd;
        static std::mt19937 gen;
        static std::uniform_int_distribution<> dis;

        SignalType type;
        union valstore{
            BitType bitValue;
            std::vector<BitType> *bitVector;
            double realValue;
        }value;
        char Bit2Char(BitType bit);
        
};
#endif // VALUE_H

