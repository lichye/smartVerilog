// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table implementation internals

#include "Vtop__pch.h"
#include "Vtop.h"
#include "Vtop___024root.h"

// FUNCTIONS
Vtop__Syms::~Vtop__Syms()
{

    // Tear down scope hierarchy
    __Vhier.remove(0, &__Vscope_fixed_relu);

}

Vtop__Syms::Vtop__Syms(VerilatedContext* contextp, const char* namep, Vtop* modelp)
    : VerilatedSyms{contextp}
    // Setup internal state of the Syms class
    , __Vm_modelp{modelp}
    // Setup module instances
    , TOP{this, namep}
{
    // Configure time unit / time precision
    _vm_contextp__->timeunit(-9);
    _vm_contextp__->timeprecision(-12);
    // Setup each module's pointers to their submodules
    // Setup each module's pointer back to symbol table (for public functions)
    TOP.__Vconfigure(true);
    // Setup scopes
    __Vscope_TOP.configure(this, name(), "TOP", "TOP", 0, VerilatedScope::SCOPE_OTHER);
    __Vscope_fixed_relu.configure(this, name(), "fixed_relu", "fixed_relu", -9, VerilatedScope::SCOPE_MODULE);

    // Set up scope hierarchy
    __Vhier.add(0, &__Vscope_fixed_relu);

    // Setup export functions
    for (int __Vfinal = 0; __Vfinal < 2; ++__Vfinal) {
        __Vscope_TOP.varInsert(__Vfinal,"clk", &(TOP.clk), false, VLVT_UINT8,VLVD_IN|VLVF_PUB_RW,0);
        __Vscope_TOP.varInsert(__Vfinal,"data_in", &(TOP.data_in), false, VLVT_UINT32,VLVD_IN|VLVF_PUB_RW,2 ,31,0 ,15,0);
        __Vscope_TOP.varInsert(__Vfinal,"data_in_ready", &(TOP.data_in_ready), false, VLVT_UINT8,VLVD_OUT|VLVF_PUB_RW,0);
        __Vscope_TOP.varInsert(__Vfinal,"data_in_valid", &(TOP.data_in_valid), false, VLVT_UINT8,VLVD_IN|VLVF_PUB_RW,0);
        __Vscope_TOP.varInsert(__Vfinal,"data_out", &(TOP.data_out), false, VLVT_UINT32,VLVD_OUT|VLVF_PUB_RW,2 ,31,0 ,15,0);
        __Vscope_TOP.varInsert(__Vfinal,"data_out_ready", &(TOP.data_out_ready), false, VLVT_UINT8,VLVD_IN|VLVF_PUB_RW,0);
        __Vscope_TOP.varInsert(__Vfinal,"data_out_valid", &(TOP.data_out_valid), false, VLVT_UINT8,VLVD_OUT|VLVF_PUB_RW,0);
        __Vscope_TOP.varInsert(__Vfinal,"rst", &(TOP.rst), false, VLVT_UINT8,VLVD_IN|VLVF_PUB_RW,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"IN_FRAC_WIDTH", const_cast<void*>(static_cast<const void*>(&(TOP.fixed_relu__DOT__IN_FRAC_WIDTH))), true, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,1 ,31,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"IN_SIZE", const_cast<void*>(static_cast<const void*>(&(TOP.fixed_relu__DOT__IN_SIZE))), true, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,1 ,31,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"IN_WIDTH", const_cast<void*>(static_cast<const void*>(&(TOP.fixed_relu__DOT__IN_WIDTH))), true, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,1 ,31,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"OUT_FRAC_WIDTH", const_cast<void*>(static_cast<const void*>(&(TOP.fixed_relu__DOT__OUT_FRAC_WIDTH))), true, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,1 ,31,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"OUT_SIZE", const_cast<void*>(static_cast<const void*>(&(TOP.fixed_relu__DOT__OUT_SIZE))), true, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,1 ,31,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"OUT_WIDTH", const_cast<void*>(static_cast<const void*>(&(TOP.fixed_relu__DOT__OUT_WIDTH))), true, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,1 ,31,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"clk", &(TOP.fixed_relu__DOT__clk), false, VLVT_UINT8,VLVD_NODIR|VLVF_PUB_RW,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"data_in", &(TOP.fixed_relu__DOT__data_in), false, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,2 ,31,0 ,15,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"data_in_ready", &(TOP.fixed_relu__DOT__data_in_ready), false, VLVT_UINT8,VLVD_NODIR|VLVF_PUB_RW,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"data_in_valid", &(TOP.fixed_relu__DOT__data_in_valid), false, VLVT_UINT8,VLVD_NODIR|VLVF_PUB_RW,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"data_out", &(TOP.fixed_relu__DOT__data_out), false, VLVT_UINT32,VLVD_NODIR|VLVF_PUB_RW,2 ,31,0 ,15,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"data_out_ready", &(TOP.fixed_relu__DOT__data_out_ready), false, VLVT_UINT8,VLVD_NODIR|VLVF_PUB_RW,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"data_out_valid", &(TOP.fixed_relu__DOT__data_out_valid), false, VLVT_UINT8,VLVD_NODIR|VLVF_PUB_RW,0);
        __Vscope_fixed_relu.varInsert(__Vfinal,"rst", &(TOP.fixed_relu__DOT__rst), false, VLVT_UINT8,VLVD_NODIR|VLVF_PUB_RW,0);
    }
}
