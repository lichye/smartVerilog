#include "VCDFile.hpp"
#include "VCDFileParser.hpp"
#include "VCDValue.hpp"
#include <iostream>  
#include <vector>    
  

int main(){

    std::cout<<"Run the test case"<<std::endl;
    
    VCDFileParser parser;

    VCDFile * trace = parser.parse_file("/home/magna/smartVerilog/myTool/Tests/VCD_files/bl_master1h.vcd");

    // if(trace == nullptr) {
    //     // Something went wrong.
    // } else {

    //     for(VCDScope * scope : *trace -> get_scopes()) {

    //         std::cout << "Scope: "  << scope ->  name  << std::endl;

    //         for(VCDSignal * signal : scope -> signals) {

    //             std::cout << "\t" << signal -> hash << "\t" 
    //                     << signal -> reference;

    //             if(signal -> size >= 1) {
    //                 std::cout << " [" << signal -> lindex << ":"<< signal -> rindex<<"]";
    //             }
                
    //             std::cout << std::endl;

    //         }
    //     }

    // }


    //Get the first signal we fancy.
    VCDSignal * mysignal = trace -> get_scope("slave_0") -> signals[6];

    //std::cout<<"Find the mysignal"<<std::endl;

    if(mysignal == nullptr){
        std::cout<<"Does not exit such file"<<std::endl;
        return -1;
    }

    // Print the value of this signal at every time step.

    for (VCDTime time : *trace -> get_timestamps()) {

        VCDValue * val = trace -> get_signal_value_at( mysignal -> hash, time);

        std::cout << "t = " << time
                << ", "   << mysignal -> reference
                << " = ";
        
        // Assumes val is not nullptr!
        switch(val -> get_type()) {
            case (VCD_SCALAR):
                std::cout << VCDValue::VCDBit2Char(val->get_value_bit());
                break;
            case (VCD_VECTOR):{
                VCDBitVector * vecval = val -> get_value_vector();
                for(auto it = vecval -> begin();
                        it != vecval -> end();
                        ++it) {
                  std::cout << VCDValue::VCDBit2Char(*it);
                }
            }
               
                break;
            case (VCD_REAL):
                std::cout << val -> get_value_real();
            default:
                break;
        }

        std::cout << std::endl;

    }

}