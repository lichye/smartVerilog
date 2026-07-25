#include "Value.h"
#include "VCDValue.hpp"
#include <iostream>
#include <random>
#include <cassert>

// std::random_device Value::rd;
// std::random_device Value::rd;
// std::mt19937 Value::gen(rd());

std::mt19937 Value::gen(std::random_device{}());
std::uniform_int_distribution<> Value::dis(0, 1);

Value::Value(){
    type = SignalType::UNKNOWNSINGALTYPES;
    value.bitValue = BitType::X;
}

Value::Value(VCDValue* vcdValue) : Value(vcdValue, 0) {}

Value::Value(VCDValue* vcdValue, int declaredWidth){
    
    if(vcdValue -> get_type() == VCDValueType::VCD_SCALAR){
        type = SignalType::BOOLEAN;
        switch (vcdValue -> get_value_bit())
        {
            case VCDBit::VCD_0:
                value.bitValue = BitType::ZERO;
                break;
            case VCDBit::VCD_1:
                value.bitValue = BitType::ONE;
                break;
            case VCDBit::VCD_X:
                value.bitValue = BitType::X;
                break;
            case VCDBit::VCD_Z:
                value.bitValue = BitType::Z;
                break;
            default:
                std::cout<<"Unknown bit value"<<std::endl;
                exit(1);
        }
    }

    else if(vcdValue -> get_type() == VCDValueType::VCD_VECTOR){
        
        type = SignalType::BITS;
        value.bitVector = new std::vector<BitType>();
        VCDBitVector * vecval = vcdValue -> get_value_vector();
        for(auto it = vecval -> begin();it != vecval -> end();++it) {
            switch (*it)
            {
                case VCDBit::VCD_0:
                    value.bitVector -> push_back(BitType::ZERO);
                    break;
                case VCDBit::VCD_1:
                    value.bitVector -> push_back(BitType::ONE);
                    break;
                case VCDBit::VCD_X:
                    value.bitVector -> push_back(BitType::X);
                    break;
                case VCDBit::VCD_Z:
                    value.bitVector -> push_back(BitType::Z);
                    break;
                default:
                    std::cout<<"Unknown bit value"<<std::endl;
                    exit(1);
            }
        }

        // Left-extend to the declared width (IEEE 1364 §18.2.1): with zeros,
        // or with the leading bit when that is x or z.
        if(declaredWidth > 1 && value.bitVector -> size() < (size_t) declaredWidth){
            BitType pad = BitType::ZERO;
            if(!value.bitVector -> empty()){
                BitType leading = value.bitVector -> front();
                if(leading == BitType::X || leading == BitType::Z)
                    pad = leading;
            }
            value.bitVector -> insert(value.bitVector -> begin(),
                                      declaredWidth - value.bitVector -> size(),
                                      pad);
        }
    }
    
    else if(vcdValue -> get_type() == VCDValueType::VCD_REAL){
        type = SignalType::DOUBLE;
        value.realValue = vcdValue->get_value_real();
    }
}

Value::~Value(){
    if(type == SignalType::BITS){
        delete value.bitVector;
    }
}

Value* Value::clone(){
    Value* value = new Value();
    value->type = type;
    if(type == SignalType::BOOLEAN){
        value->value.bitValue = this->value.bitValue;
    }
    else if(type == SignalType::BITS){
        value->value.bitVector = new std::vector<BitType>();
        for(auto &bit : *this->value.bitVector){
            value->value.bitVector->push_back(bit);
        }
    }
    else if(type == SignalType::DOUBLE){
        value->value.realValue = this->value.realValue;
    }
    return value;
}
std::string Value::toString(){
    if(type == SignalType::BOOLEAN){
        if(value.bitValue == BitType::ZERO)
            return "false";
        else
            return "true";
    }
    else if(type == SignalType::BITS){
        std::string str="#b";
        for(auto &bit : *value.bitVector){
            str += Bit2Char(bit);
        }
        return str;
    }
    else if(type == SignalType::DOUBLE){
        return std::to_string(value.realValue);
    }
    else{
        return "unknownType";
    }   
}

std::string Value::toSyGusString(){
    if(type == SignalType::BOOLEAN){
        if(value.bitValue == BitType::ZERO)
            return "false";
        else if (value.bitValue == BitType::ONE)
            return "true";
        else
            return "unknown_Bool";
    }
    else if(type == SignalType::BITS){
        assert(value.bitVector != nullptr);
        if(value.bitVector->size() == 1){
            std::string str = "";
            switch (value.bitVector->at(0))
            {
                case BitType::ZERO:
                    str += "false";
                    break;
                case BitType::ONE:
                    str += "true";
                    break;
                case BitType::X:
                    str += "unknown_bits";
                    break;
                case BitType::Z:
                    str += "unknown_bits";
                    break;
                default:
                    break;
            }
            return str;
        }
        std::string str="#b";
        for(auto &bit : *value.bitVector){
            switch (bit)
            {
            case BitType::ZERO:
                str += "0";
                break;
            case BitType::ONE:
                str += "1";
                break;
            case BitType::X:
                str += "X";
                break;
            case BitType::Z:
                str += "Z";
                break;
            default:
                break;
            }
        }
        return str;
    }
    else if(type == SignalType::DOUBLE){
        return std::to_string(value.realValue);
    }
    else{
        return "unknown_value";
    }   
}

std::string Value::toVerilogString(){
    if(type == SignalType::BOOLEAN){
        if(value.bitValue == BitType::ZERO)
            return "0";
        else
            return "1";
    }
    else if(type == SignalType::BITS){
        std::string str = std::to_string(value.bitVector->size()) + "'b";
        for(auto &bit : *value.bitVector){
            str += Bit2Char(bit);
        }
        return str;
    }
    else if(type == SignalType::DOUBLE){
        return std::to_string(value.realValue);
    }
    else{
        return "unknown_value";
    }   
}

bool Value::isUndefined(){
    // Z counts as undefined as well as X. toSyGusString() prints BOTH as the
    // placeholder `unknown_Bool` / `unknown_bits`, so treating z as a defined
    // value let that token reach the SyGuS file, where it is not valid syntax
    // and costs the solver the whole problem. (Verilator's 2-state traces have
    // neither; Icarus Verilog reports z for an undriven net.)
    if(type == SignalType::BOOLEAN){
        return value.bitValue == BitType::X || value.bitValue == BitType::Z;
    }
    else if(type == SignalType::BITS){
        for(auto &bit : *value.bitVector){
            if(bit == BitType::X || bit == BitType::Z){
                return true;
            }
        }
        return false;
    }
    else{
        return true;
    }
}

char Value::Bit2Char(BitType bit){
    switch(bit){
        case BitType::ZERO:
            return '0';
        case BitType::ONE:
            return '1';
        case BitType::X:
            return 'X';
        case BitType::Z:
            return 'Z';
        default:
            return 'U';
    }
}

void Value::setSeed(int seed){
    gen.seed(seed);
}

Value* Value::makeXValue(SignalType type,int bitWidth){
    Value* value = new Value();
    value->type = type;
    if(type == SignalType::BOOLEAN){
        value->value.bitValue = BitType::X;
    }
    else if(type == SignalType::BITS){
        value->value.bitVector = new std::vector<BitType>();
        for(int i = 0; i < bitWidth; i++){
            value->value.bitVector->push_back(BitType::X);
            //std::cout<<"The bit vector size is: "<<value->value.bitVector->size()<<std::endl;
        }
    }
    else if(type == SignalType::DOUBLE){
        value->value.realValue = 0.0;
    }
    return value;
}

Value* Value::makeRandomValue(SignalType type,int bitWidth){
    Value* value = new Value();
    
    value->type = type;
    if(type == SignalType::BOOLEAN){
        value->value.bitValue = dis(gen) == 0 ? BitType::ZERO : BitType::ONE;
    }
    else if(type == SignalType::BITS){
        value->value.bitVector = new std::vector<BitType>();
        for(int i = 0; i < bitWidth; i++){
            value->value.bitVector->push_back(dis(gen) == 0 ? BitType::ZERO : BitType::ONE);
        }
    }
    else if(type == SignalType::DOUBLE){
        value->value.realValue = dis(gen);
    }
    return value;
}

Value* Value::makeNewValue(Value* oldValue,SignalType type,int bitWidth){
    Value* newValue = new Value();
    newValue->type = type;
    if(type == SignalType::BOOLEAN){
        if(oldValue->value.bitValue == BitType::ZERO)
            newValue->value.bitValue = BitType::ONE;
        else if(oldValue->value.bitValue == BitType::ONE)
            newValue->value.bitValue = BitType::ZERO;
        else
            newValue->value.bitValue = BitType::X;
    }
    else if(type == SignalType::BITS){
        newValue->value.bitVector = new std::vector<BitType>();
        for(int i = 0; i < bitWidth; i++){
            newValue->value.bitVector->push_back(dis(gen) == 0 ? BitType::ZERO : BitType::ONE);
        }
    }
    else if(type == SignalType::DOUBLE){
        newValue->value.realValue = dis(gen);
    }
    return newValue;
}

