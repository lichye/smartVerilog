#include "Value.h"
#include "VCDValue.hpp"
#include <iostream>

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

std::string Value::toString(){
    if(type == SignalType::BOOLEAN){
        return std::string(1,Bit2Char(value.bitValue));
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