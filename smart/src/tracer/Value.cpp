#include "Value.h"
#include "VCDValue.hpp"
#include <iostream>
#include <random>
#include <cassert>

std::random_device Value::rd;
std::mt19937 Value::gen(rd());
std::uniform_int_distribution<> Value::dis(0, 1);

Value::Value(){
    type = SignalType::UNKNOWNSINGALTYPES;
    value.bitValue = BitType::X;
}

Value::Value(VCDValue* vcdValue){
    
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
        return "unknown";
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
    if(type == SignalType::BOOLEAN){
        return value.bitValue == BitType::X;
    }
    else if(type == SignalType::BITS){
        for(auto &bit : *value.bitVector){
            if(bit == BitType::X){
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

