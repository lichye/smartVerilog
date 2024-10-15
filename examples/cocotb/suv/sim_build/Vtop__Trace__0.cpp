// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals
#include "verilated_vcd_c.h"
#include "Vtop__Syms.h"


void Vtop___024root__trace_chg_0_sub_0(Vtop___024root* vlSelf, VerilatedVcd::Buffer* bufp);

void Vtop___024root__trace_chg_0(void* voidSelf, VerilatedVcd::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_chg_0\n"); );
    // Init
    Vtop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<Vtop___024root*>(voidSelf);
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    if (VL_UNLIKELY(!vlSymsp->__Vm_activity)) return;
    // Body
    Vtop___024root__trace_chg_0_sub_0((&vlSymsp->TOP), bufp);
}

void Vtop___024root__trace_chg_0_sub_0(Vtop___024root* vlSelf, VerilatedVcd::Buffer* bufp) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_chg_0_sub_0\n"); );
    // Init
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode + 1);
    // Body
    bufp->chgBit(oldp+0,(vlSelf->rst));
    bufp->chgBit(oldp+1,(vlSelf->clk));
    bufp->chgIData(oldp+2,(vlSelf->data_in[0]),32);
    bufp->chgIData(oldp+3,(vlSelf->data_in[1]),32);
    bufp->chgIData(oldp+4,(vlSelf->data_in[2]),32);
    bufp->chgIData(oldp+5,(vlSelf->data_in[3]),32);
    bufp->chgIData(oldp+6,(vlSelf->data_in[4]),32);
    bufp->chgIData(oldp+7,(vlSelf->data_in[5]),32);
    bufp->chgIData(oldp+8,(vlSelf->data_in[6]),32);
    bufp->chgIData(oldp+9,(vlSelf->data_in[7]),32);
    bufp->chgIData(oldp+10,(vlSelf->data_in[8]),32);
    bufp->chgIData(oldp+11,(vlSelf->data_in[9]),32);
    bufp->chgIData(oldp+12,(vlSelf->data_in[10]),32);
    bufp->chgIData(oldp+13,(vlSelf->data_in[11]),32);
    bufp->chgIData(oldp+14,(vlSelf->data_in[12]),32);
    bufp->chgIData(oldp+15,(vlSelf->data_in[13]),32);
    bufp->chgIData(oldp+16,(vlSelf->data_in[14]),32);
    bufp->chgIData(oldp+17,(vlSelf->data_in[15]),32);
    bufp->chgIData(oldp+18,(vlSelf->data_out[0]),32);
    bufp->chgIData(oldp+19,(vlSelf->data_out[1]),32);
    bufp->chgIData(oldp+20,(vlSelf->data_out[2]),32);
    bufp->chgIData(oldp+21,(vlSelf->data_out[3]),32);
    bufp->chgIData(oldp+22,(vlSelf->data_out[4]),32);
    bufp->chgIData(oldp+23,(vlSelf->data_out[5]),32);
    bufp->chgIData(oldp+24,(vlSelf->data_out[6]),32);
    bufp->chgIData(oldp+25,(vlSelf->data_out[7]),32);
    bufp->chgIData(oldp+26,(vlSelf->data_out[8]),32);
    bufp->chgIData(oldp+27,(vlSelf->data_out[9]),32);
    bufp->chgIData(oldp+28,(vlSelf->data_out[10]),32);
    bufp->chgIData(oldp+29,(vlSelf->data_out[11]),32);
    bufp->chgIData(oldp+30,(vlSelf->data_out[12]),32);
    bufp->chgIData(oldp+31,(vlSelf->data_out[13]),32);
    bufp->chgIData(oldp+32,(vlSelf->data_out[14]),32);
    bufp->chgIData(oldp+33,(vlSelf->data_out[15]),32);
    bufp->chgBit(oldp+34,(vlSelf->data_in_valid));
    bufp->chgBit(oldp+35,(vlSelf->data_in_ready));
    bufp->chgBit(oldp+36,(vlSelf->data_out_valid));
    bufp->chgBit(oldp+37,(vlSelf->data_out_ready));
    bufp->chgBit(oldp+38,(vlSelf->fixed_relu__DOT__rst));
    bufp->chgBit(oldp+39,(vlSelf->fixed_relu__DOT__clk));
    bufp->chgIData(oldp+40,(vlSelf->fixed_relu__DOT__data_in[0]),32);
    bufp->chgIData(oldp+41,(vlSelf->fixed_relu__DOT__data_in[1]),32);
    bufp->chgIData(oldp+42,(vlSelf->fixed_relu__DOT__data_in[2]),32);
    bufp->chgIData(oldp+43,(vlSelf->fixed_relu__DOT__data_in[3]),32);
    bufp->chgIData(oldp+44,(vlSelf->fixed_relu__DOT__data_in[4]),32);
    bufp->chgIData(oldp+45,(vlSelf->fixed_relu__DOT__data_in[5]),32);
    bufp->chgIData(oldp+46,(vlSelf->fixed_relu__DOT__data_in[6]),32);
    bufp->chgIData(oldp+47,(vlSelf->fixed_relu__DOT__data_in[7]),32);
    bufp->chgIData(oldp+48,(vlSelf->fixed_relu__DOT__data_in[8]),32);
    bufp->chgIData(oldp+49,(vlSelf->fixed_relu__DOT__data_in[9]),32);
    bufp->chgIData(oldp+50,(vlSelf->fixed_relu__DOT__data_in[10]),32);
    bufp->chgIData(oldp+51,(vlSelf->fixed_relu__DOT__data_in[11]),32);
    bufp->chgIData(oldp+52,(vlSelf->fixed_relu__DOT__data_in[12]),32);
    bufp->chgIData(oldp+53,(vlSelf->fixed_relu__DOT__data_in[13]),32);
    bufp->chgIData(oldp+54,(vlSelf->fixed_relu__DOT__data_in[14]),32);
    bufp->chgIData(oldp+55,(vlSelf->fixed_relu__DOT__data_in[15]),32);
    bufp->chgIData(oldp+56,(vlSelf->fixed_relu__DOT__data_out[0]),32);
    bufp->chgIData(oldp+57,(vlSelf->fixed_relu__DOT__data_out[1]),32);
    bufp->chgIData(oldp+58,(vlSelf->fixed_relu__DOT__data_out[2]),32);
    bufp->chgIData(oldp+59,(vlSelf->fixed_relu__DOT__data_out[3]),32);
    bufp->chgIData(oldp+60,(vlSelf->fixed_relu__DOT__data_out[4]),32);
    bufp->chgIData(oldp+61,(vlSelf->fixed_relu__DOT__data_out[5]),32);
    bufp->chgIData(oldp+62,(vlSelf->fixed_relu__DOT__data_out[6]),32);
    bufp->chgIData(oldp+63,(vlSelf->fixed_relu__DOT__data_out[7]),32);
    bufp->chgIData(oldp+64,(vlSelf->fixed_relu__DOT__data_out[8]),32);
    bufp->chgIData(oldp+65,(vlSelf->fixed_relu__DOT__data_out[9]),32);
    bufp->chgIData(oldp+66,(vlSelf->fixed_relu__DOT__data_out[10]),32);
    bufp->chgIData(oldp+67,(vlSelf->fixed_relu__DOT__data_out[11]),32);
    bufp->chgIData(oldp+68,(vlSelf->fixed_relu__DOT__data_out[12]),32);
    bufp->chgIData(oldp+69,(vlSelf->fixed_relu__DOT__data_out[13]),32);
    bufp->chgIData(oldp+70,(vlSelf->fixed_relu__DOT__data_out[14]),32);
    bufp->chgIData(oldp+71,(vlSelf->fixed_relu__DOT__data_out[15]),32);
    bufp->chgBit(oldp+72,(vlSelf->fixed_relu__DOT__data_in_valid));
    bufp->chgBit(oldp+73,(vlSelf->fixed_relu__DOT__data_in_ready));
    bufp->chgBit(oldp+74,(vlSelf->fixed_relu__DOT__data_out_valid));
    bufp->chgBit(oldp+75,(vlSelf->fixed_relu__DOT__data_out_ready));
}

void Vtop___024root__trace_cleanup(void* voidSelf, VerilatedVcd* /*unused*/) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_cleanup\n"); );
    // Init
    Vtop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<Vtop___024root*>(voidSelf);
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VlUnpacked<CData/*0:0*/, 1> __Vm_traceActivity;
    for (int __Vi0 = 0; __Vi0 < 1; ++__Vi0) {
        __Vm_traceActivity[__Vi0] = 0;
    }
    // Body
    vlSymsp->__Vm_activity = false;
    __Vm_traceActivity[0U] = 0U;
}
