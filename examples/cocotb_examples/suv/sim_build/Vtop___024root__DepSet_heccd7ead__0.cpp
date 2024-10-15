// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop.h for the primary calling header

#include "Vtop__pch.h"
#include "Vtop___024root.h"

VL_INLINE_OPT void Vtop___024root___ico_sequent__TOP__0(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___ico_sequent__TOP__0\n"); );
    // Body
    vlSelf->fixed_relu__DOT__rst = vlSelf->rst;
    vlSelf->fixed_relu__DOT__clk = vlSelf->clk;
    vlSelf->fixed_relu__DOT__data_in_valid = vlSelf->data_in_valid;
    vlSelf->fixed_relu__DOT__data_out_ready = vlSelf->data_out_ready;
    vlSelf->data_out_valid = vlSelf->data_in_valid;
    vlSelf->data_in_ready = vlSelf->data_out_ready;
    vlSelf->fixed_relu__DOT__data_in[0xfU] = vlSelf->data_in
        [0xfU];
    vlSelf->fixed_relu__DOT__data_in[0xeU] = vlSelf->data_in
        [0xeU];
    vlSelf->fixed_relu__DOT__data_in[0xdU] = vlSelf->data_in
        [0xdU];
    vlSelf->fixed_relu__DOT__data_in[0xcU] = vlSelf->data_in
        [0xcU];
    vlSelf->fixed_relu__DOT__data_in[0xbU] = vlSelf->data_in
        [0xbU];
    vlSelf->fixed_relu__DOT__data_in[0xaU] = vlSelf->data_in
        [0xaU];
    vlSelf->fixed_relu__DOT__data_in[9U] = vlSelf->data_in
        [9U];
    vlSelf->fixed_relu__DOT__data_in[8U] = vlSelf->data_in
        [8U];
    vlSelf->fixed_relu__DOT__data_in[7U] = vlSelf->data_in
        [7U];
    vlSelf->fixed_relu__DOT__data_in[6U] = vlSelf->data_in
        [6U];
    vlSelf->fixed_relu__DOT__data_in[5U] = vlSelf->data_in
        [5U];
    vlSelf->fixed_relu__DOT__data_in[4U] = vlSelf->data_in
        [4U];
    vlSelf->fixed_relu__DOT__data_in[3U] = vlSelf->data_in
        [3U];
    vlSelf->fixed_relu__DOT__data_in[2U] = vlSelf->data_in
        [2U];
    vlSelf->fixed_relu__DOT__data_in[1U] = vlSelf->data_in
        [1U];
    vlSelf->fixed_relu__DOT__data_in[0U] = vlSelf->data_in
        [0U];
    vlSelf->fixed_relu__DOT__data_out[0U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [0U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [0U]);
    vlSelf->fixed_relu__DOT__data_out[1U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [1U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [1U]);
    vlSelf->fixed_relu__DOT__data_out[2U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [2U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [2U]);
    vlSelf->fixed_relu__DOT__data_out[3U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [3U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [3U]);
    vlSelf->fixed_relu__DOT__data_out[4U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [4U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [4U]);
    vlSelf->fixed_relu__DOT__data_out[5U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [5U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [5U]);
    vlSelf->fixed_relu__DOT__data_out[6U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [6U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [6U]);
    vlSelf->fixed_relu__DOT__data_out[7U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [7U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [7U]);
    vlSelf->fixed_relu__DOT__data_out[8U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [8U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [8U]);
    vlSelf->fixed_relu__DOT__data_out[9U] = (VL_GTES_III(32, 0U, 
                                                         vlSelf->data_in
                                                         [9U])
                                              ? 0U : 
                                             vlSelf->data_in
                                             [9U]);
    vlSelf->fixed_relu__DOT__data_out[0xaU] = (VL_GTES_III(32, 0U, 
                                                           vlSelf->data_in
                                                           [0xaU])
                                                ? 0U
                                                : vlSelf->data_in
                                               [0xaU]);
    vlSelf->fixed_relu__DOT__data_out[0xbU] = (VL_GTES_III(32, 0U, 
                                                           vlSelf->data_in
                                                           [0xbU])
                                                ? 0U
                                                : vlSelf->data_in
                                               [0xbU]);
    vlSelf->fixed_relu__DOT__data_out[0xcU] = (VL_GTES_III(32, 0U, 
                                                           vlSelf->data_in
                                                           [0xcU])
                                                ? 0U
                                                : vlSelf->data_in
                                               [0xcU]);
    vlSelf->fixed_relu__DOT__data_out[0xdU] = (VL_GTES_III(32, 0U, 
                                                           vlSelf->data_in
                                                           [0xdU])
                                                ? 0U
                                                : vlSelf->data_in
                                               [0xdU]);
    vlSelf->fixed_relu__DOT__data_out[0xeU] = (VL_GTES_III(32, 0U, 
                                                           vlSelf->data_in
                                                           [0xeU])
                                                ? 0U
                                                : vlSelf->data_in
                                               [0xeU]);
    vlSelf->fixed_relu__DOT__data_out[0xfU] = (VL_GTES_III(32, 0U, 
                                                           vlSelf->data_in
                                                           [0xfU])
                                                ? 0U
                                                : vlSelf->data_in
                                               [0xfU]);
    vlSelf->fixed_relu__DOT__data_out_valid = vlSelf->data_out_valid;
    vlSelf->fixed_relu__DOT__data_in_ready = vlSelf->data_in_ready;
    vlSelf->data_out[0xfU] = vlSelf->fixed_relu__DOT__data_out
        [0xfU];
    vlSelf->data_out[0xeU] = vlSelf->fixed_relu__DOT__data_out
        [0xeU];
    vlSelf->data_out[0xdU] = vlSelf->fixed_relu__DOT__data_out
        [0xdU];
    vlSelf->data_out[0xcU] = vlSelf->fixed_relu__DOT__data_out
        [0xcU];
    vlSelf->data_out[0xbU] = vlSelf->fixed_relu__DOT__data_out
        [0xbU];
    vlSelf->data_out[0xaU] = vlSelf->fixed_relu__DOT__data_out
        [0xaU];
    vlSelf->data_out[9U] = vlSelf->fixed_relu__DOT__data_out
        [9U];
    vlSelf->data_out[8U] = vlSelf->fixed_relu__DOT__data_out
        [8U];
    vlSelf->data_out[7U] = vlSelf->fixed_relu__DOT__data_out
        [7U];
    vlSelf->data_out[6U] = vlSelf->fixed_relu__DOT__data_out
        [6U];
    vlSelf->data_out[5U] = vlSelf->fixed_relu__DOT__data_out
        [5U];
    vlSelf->data_out[4U] = vlSelf->fixed_relu__DOT__data_out
        [4U];
    vlSelf->data_out[3U] = vlSelf->fixed_relu__DOT__data_out
        [3U];
    vlSelf->data_out[2U] = vlSelf->fixed_relu__DOT__data_out
        [2U];
    vlSelf->data_out[1U] = vlSelf->fixed_relu__DOT__data_out
        [1U];
    vlSelf->data_out[0U] = vlSelf->fixed_relu__DOT__data_out
        [0U];
}

void Vtop___024root___eval_ico(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_ico\n"); );
    // Body
    if ((1ULL & vlSelf->__VicoTriggered.word(0U))) {
        Vtop___024root___ico_sequent__TOP__0(vlSelf);
    }
}

void Vtop___024root___eval_triggers__ico(Vtop___024root* vlSelf);

bool Vtop___024root___eval_phase__ico(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_phase__ico\n"); );
    // Init
    CData/*0:0*/ __VicoExecute;
    // Body
    Vtop___024root___eval_triggers__ico(vlSelf);
    __VicoExecute = vlSelf->__VicoTriggered.any();
    if (__VicoExecute) {
        Vtop___024root___eval_ico(vlSelf);
    }
    return (__VicoExecute);
}

void Vtop___024root___eval_act(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_act\n"); );
}

void Vtop___024root___eval_nba(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_nba\n"); );
}

void Vtop___024root___eval_triggers__act(Vtop___024root* vlSelf);

bool Vtop___024root___eval_phase__act(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_phase__act\n"); );
    // Init
    VlTriggerVec<0> __VpreTriggered;
    CData/*0:0*/ __VactExecute;
    // Body
    Vtop___024root___eval_triggers__act(vlSelf);
    __VactExecute = vlSelf->__VactTriggered.any();
    if (__VactExecute) {
        __VpreTriggered.andNot(vlSelf->__VactTriggered, vlSelf->__VnbaTriggered);
        vlSelf->__VnbaTriggered.thisOr(vlSelf->__VactTriggered);
        Vtop___024root___eval_act(vlSelf);
    }
    return (__VactExecute);
}

bool Vtop___024root___eval_phase__nba(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_phase__nba\n"); );
    // Init
    CData/*0:0*/ __VnbaExecute;
    // Body
    __VnbaExecute = vlSelf->__VnbaTriggered.any();
    if (__VnbaExecute) {
        Vtop___024root___eval_nba(vlSelf);
        vlSelf->__VnbaTriggered.clear();
    }
    return (__VnbaExecute);
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__ico(Vtop___024root* vlSelf);
#endif  // VL_DEBUG
#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__nba(Vtop___024root* vlSelf);
#endif  // VL_DEBUG
#ifdef VL_DEBUG
VL_ATTR_COLD void Vtop___024root___dump_triggers__act(Vtop___024root* vlSelf);
#endif  // VL_DEBUG

void Vtop___024root___eval(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval\n"); );
    // Init
    IData/*31:0*/ __VicoIterCount;
    CData/*0:0*/ __VicoContinue;
    IData/*31:0*/ __VnbaIterCount;
    CData/*0:0*/ __VnbaContinue;
    // Body
    __VicoIterCount = 0U;
    vlSelf->__VicoFirstIteration = 1U;
    __VicoContinue = 1U;
    while (__VicoContinue) {
        if (VL_UNLIKELY((0x64U < __VicoIterCount))) {
#ifdef VL_DEBUG
            Vtop___024root___dump_triggers__ico(vlSelf);
#endif
            VL_FATAL_MT("/home/magna/smartVerilog/examples/cocotb_examples/suv/suv.sv", 2, "", "Input combinational region did not converge.");
        }
        __VicoIterCount = ((IData)(1U) + __VicoIterCount);
        __VicoContinue = 0U;
        if (Vtop___024root___eval_phase__ico(vlSelf)) {
            __VicoContinue = 1U;
        }
        vlSelf->__VicoFirstIteration = 0U;
    }
    __VnbaIterCount = 0U;
    __VnbaContinue = 1U;
    while (__VnbaContinue) {
        if (VL_UNLIKELY((0x64U < __VnbaIterCount))) {
#ifdef VL_DEBUG
            Vtop___024root___dump_triggers__nba(vlSelf);
#endif
            VL_FATAL_MT("/home/magna/smartVerilog/examples/cocotb_examples/suv/suv.sv", 2, "", "NBA region did not converge.");
        }
        __VnbaIterCount = ((IData)(1U) + __VnbaIterCount);
        __VnbaContinue = 0U;
        vlSelf->__VactIterCount = 0U;
        vlSelf->__VactContinue = 1U;
        while (vlSelf->__VactContinue) {
            if (VL_UNLIKELY((0x64U < vlSelf->__VactIterCount))) {
#ifdef VL_DEBUG
                Vtop___024root___dump_triggers__act(vlSelf);
#endif
                VL_FATAL_MT("/home/magna/smartVerilog/examples/cocotb_examples/suv/suv.sv", 2, "", "Active region did not converge.");
            }
            vlSelf->__VactIterCount = ((IData)(1U) 
                                       + vlSelf->__VactIterCount);
            vlSelf->__VactContinue = 0U;
            if (Vtop___024root___eval_phase__act(vlSelf)) {
                vlSelf->__VactContinue = 1U;
            }
        }
        if (Vtop___024root___eval_phase__nba(vlSelf)) {
            __VnbaContinue = 1U;
        }
    }
}

#ifdef VL_DEBUG
void Vtop___024root___eval_debug_assertions(Vtop___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root___eval_debug_assertions\n"); );
    // Body
    if (VL_UNLIKELY((vlSelf->rst & 0xfeU))) {
        Verilated::overWidthError("rst");}
    if (VL_UNLIKELY((vlSelf->clk & 0xfeU))) {
        Verilated::overWidthError("clk");}
    if (VL_UNLIKELY((vlSelf->data_in_valid & 0xfeU))) {
        Verilated::overWidthError("data_in_valid");}
    if (VL_UNLIKELY((vlSelf->data_out_ready & 0xfeU))) {
        Verilated::overWidthError("data_out_ready");}
}
#endif  // VL_DEBUG
