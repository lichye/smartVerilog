#if !defined(VALUE_H)
#define VALUE_H

#include "VCDValue.hpp"
#include "Utils.h"
#include <random>

class Value{
    public:
        static Value* makeXValue(SignalType,int);
        static Value* makeRandomValue(SignalType,int);
        static Value* makeNewValue(Value*,SignalType,int);
        static void setSeed(int);

        Value(VCDValue* value);
        // `width` is the signal's declared width. VCD value changes may drop
        // leading bits (IEEE 1364 §18.2.1), so a value has to be widened
        // against the declaration or it ends up the wrong TYPE — an 8-bit
        // signal whose trace says `b0` would otherwise become a 1-bit Bool.
        Value(VCDValue* value, int width);
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

