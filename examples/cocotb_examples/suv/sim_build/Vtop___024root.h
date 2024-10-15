// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vtop.h for the primary calling header

#ifndef VERILATED_VTOP___024ROOT_H_
#define VERILATED_VTOP___024ROOT_H_  // guard

#include "verilated.h"


class Vtop__Syms;

class alignas(VL_CACHE_LINE_BYTES) Vtop___024root final : public VerilatedModule {
  public:

    // DESIGN SPECIFIC STATE
    VL_IN8(rst,0,0);
    VL_IN8(clk,0,0);
    VL_IN8(data_in_valid,0,0);
    VL_OUT8(data_in_ready,0,0);
    VL_OUT8(data_out_valid,0,0);
    VL_IN8(data_out_ready,0,0);
    CData/*0:0*/ fixed_relu__DOT__rst;
    CData/*0:0*/ fixed_relu__DOT__clk;
    CData/*0:0*/ fixed_relu__DOT__data_in_valid;
    CData/*0:0*/ fixed_relu__DOT__data_in_ready;
    CData/*0:0*/ fixed_relu__DOT__data_out_valid;
    CData/*0:0*/ fixed_relu__DOT__data_out_ready;
    CData/*0:0*/ __VstlFirstIteration;
    CData/*0:0*/ __VicoFirstIteration;
    CData/*0:0*/ __VactContinue;
    IData/*31:0*/ __VactIterCount;
    VL_IN(data_in[16],31,0);
    VL_OUT(data_out[16],31,0);
    VlUnpacked<IData/*31:0*/, 16> fixed_relu__DOT__data_in;
    VlUnpacked<IData/*31:0*/, 16> fixed_relu__DOT__data_out;
    VlTriggerVec<1> __VstlTriggered;
    VlTriggerVec<1> __VicoTriggered;
    VlTriggerVec<0> __VactTriggered;
    VlTriggerVec<0> __VnbaTriggered;

    // INTERNAL VARIABLES
    Vtop__Syms* const vlSymsp;

    // PARAMETERS
    static constexpr IData/*31:0*/ fixed_relu__DOT__IN_WIDTH = 0x00000020U;
    static constexpr IData/*31:0*/ fixed_relu__DOT__IN_FRAC_WIDTH = 0U;
    static constexpr IData/*31:0*/ fixed_relu__DOT__OUT_WIDTH = 8U;
    static constexpr IData/*31:0*/ fixed_relu__DOT__OUT_FRAC_WIDTH = 0U;
    static constexpr IData/*31:0*/ fixed_relu__DOT__OUT_SIZE = 0U;
    static constexpr IData/*31:0*/ fixed_relu__DOT__IN_SIZE = 0x00000010U;

    // CONSTRUCTORS
    Vtop___024root(Vtop__Syms* symsp, const char* v__name);
    ~Vtop___024root();
    VL_UNCOPYABLE(Vtop___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};


#endif  // guard
