// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals
#include "verilated_vcd_c.h"
#include "Vtop__Syms.h"


VL_ATTR_COLD void Vtop___024root__trace_init_sub__TOP__0(Vtop___024root* vlSelf, VerilatedVcd* tracep) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_init_sub__TOP__0\n"); );
    // Init
    const int c = vlSymsp->__Vm_baseCode;
    // Body
    tracep->declBit(c+1,0,"rst",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+2,0,"clk",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->pushPrefix("data_in", VerilatedTracePrefixType::ARRAY_UNPACKED);
    for (int i = 0; i < 16; ++i) {
        tracep->declBus(c+3+i*1,0,"",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, true,(i+0), 31,0);
    }
    tracep->popPrefix();
    tracep->pushPrefix("data_out", VerilatedTracePrefixType::ARRAY_UNPACKED);
    for (int i = 0; i < 16; ++i) {
        tracep->declBus(c+19+i*1,0,"",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, true,(i+0), 31,0);
    }
    tracep->popPrefix();
    tracep->declBit(c+35,0,"data_in_valid",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+36,0,"data_in_ready",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+37,0,"data_out_valid",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+38,0,"data_out_ready",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->pushPrefix("fixed_relu", VerilatedTracePrefixType::SCOPE_MODULE);
    tracep->declBus(c+77,0,"IN_WIDTH",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::PARAMETER, VerilatedTraceSigType::LOGIC, false,-1, 31,0);
    tracep->declBus(c+78,0,"IN_FRAC_WIDTH",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::PARAMETER, VerilatedTraceSigType::LOGIC, false,-1, 31,0);
    tracep->declBus(c+79,0,"OUT_WIDTH",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::PARAMETER, VerilatedTraceSigType::LOGIC, false,-1, 31,0);
    tracep->declBus(c+78,0,"OUT_FRAC_WIDTH",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::PARAMETER, VerilatedTraceSigType::LOGIC, false,-1, 31,0);
    tracep->declBus(c+78,0,"OUT_SIZE",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::PARAMETER, VerilatedTraceSigType::LOGIC, false,-1, 31,0);
    tracep->declBus(c+80,0,"IN_SIZE",-1, VerilatedTraceSigDirection::NONE, VerilatedTraceSigKind::PARAMETER, VerilatedTraceSigType::LOGIC, false,-1, 31,0);
    tracep->declBit(c+39,0,"rst",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+40,0,"clk",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->pushPrefix("data_in", VerilatedTracePrefixType::ARRAY_UNPACKED);
    for (int i = 0; i < 16; ++i) {
        tracep->declBus(c+41+i*1,0,"",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, true,(i+0), 31,0);
    }
    tracep->popPrefix();
    tracep->pushPrefix("data_out", VerilatedTracePrefixType::ARRAY_UNPACKED);
    for (int i = 0; i < 16; ++i) {
        tracep->declBus(c+57+i*1,0,"",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, true,(i+0), 31,0);
    }
    tracep->popPrefix();
    tracep->declBit(c+73,0,"data_in_valid",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+74,0,"data_in_ready",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+75,0,"data_out_valid",-1, VerilatedTraceSigDirection::OUTPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->declBit(c+76,0,"data_out_ready",-1, VerilatedTraceSigDirection::INPUT, VerilatedTraceSigKind::WIRE, VerilatedTraceSigType::LOGIC, false,-1);
    tracep->popPrefix();
}

VL_ATTR_COLD void Vtop___024root__trace_init_top(Vtop___024root* vlSelf, VerilatedVcd* tracep) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_init_top\n"); );
    // Body
    Vtop___024root__trace_init_sub__TOP__0(vlSelf, tracep);
}

VL_ATTR_COLD void Vtop___024root__trace_const_0(void* voidSelf, VerilatedVcd::Buffer* bufp);
VL_ATTR_COLD void Vtop___024root__trace_full_0(void* voidSelf, VerilatedVcd::Buffer* bufp);
void Vtop___024root__trace_chg_0(void* voidSelf, VerilatedVcd::Buffer* bufp);
void Vtop___024root__trace_cleanup(void* voidSelf, VerilatedVcd* /*unused*/);

VL_ATTR_COLD void Vtop___024root__trace_register(Vtop___024root* vlSelf, VerilatedVcd* tracep) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_register\n"); );
    // Body
    tracep->addConstCb(&Vtop___024root__trace_const_0, 0U, vlSelf);
    tracep->addFullCb(&Vtop___024root__trace_full_0, 0U, vlSelf);
    tracep->addChgCb(&Vtop___024root__trace_chg_0, 0U, vlSelf);
    tracep->addCleanupCb(&Vtop___024root__trace_cleanup, vlSelf);
}

VL_ATTR_COLD void Vtop___024root__trace_const_0_sub_0(Vtop___024root* vlSelf, VerilatedVcd::Buffer* bufp);

VL_ATTR_COLD void Vtop___024root__trace_const_0(void* voidSelf, VerilatedVcd::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_const_0\n"); );
    // Init
    Vtop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<Vtop___024root*>(voidSelf);
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    // Body
    Vtop___024root__trace_const_0_sub_0((&vlSymsp->TOP), bufp);
}

VL_ATTR_COLD void Vtop___024root__trace_const_0_sub_0(Vtop___024root* vlSelf, VerilatedVcd::Buffer* bufp) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_const_0_sub_0\n"); );
    // Init
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode);
    // Body
    bufp->fullIData(oldp+77,(0x20U),32);
    bufp->fullIData(oldp+78,(0U),32);
    bufp->fullIData(oldp+79,(8U),32);
    bufp->fullIData(oldp+80,(0x10U),32);
}

VL_ATTR_COLD void Vtop___024root__trace_full_0_sub_0(Vtop___024root* vlSelf, VerilatedVcd::Buffer* bufp);

VL_ATTR_COLD void Vtop___024root__trace_full_0(void* voidSelf, VerilatedVcd::Buffer* bufp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_full_0\n"); );
    // Init
    Vtop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<Vtop___024root*>(voidSelf);
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    // Body
    Vtop___024root__trace_full_0_sub_0((&vlSymsp->TOP), bufp);
}

VL_ATTR_COLD void Vtop___024root__trace_full_0_sub_0(Vtop___024root* vlSelf, VerilatedVcd::Buffer* bufp) {
    if (false && vlSelf) {}  // Prevent unused
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop___024root__trace_full_0_sub_0\n"); );
    // Init
    uint32_t* const oldp VL_ATTR_UNUSED = bufp->oldp(vlSymsp->__Vm_baseCode);
    // Body
    bufp->fullBit(oldp+1,(vlSelf->rst));
    bufp->fullBit(oldp+2,(vlSelf->clk));
    bufp->fullIData(oldp+3,(vlSelf->data_in[0]),32);
    bufp->fullIData(oldp+4,(vlSelf->data_in[1]),32);
    bufp->fullIData(oldp+5,(vlSelf->data_in[2]),32);
    bufp->fullIData(oldp+6,(vlSelf->data_in[3]),32);
    bufp->fullIData(oldp+7,(vlSelf->data_in[4]),32);
    bufp->fullIData(oldp+8,(vlSelf->data_in[5]),32);
    bufp->fullIData(oldp+9,(vlSelf->data_in[6]),32);
    bufp->fullIData(oldp+10,(vlSelf->data_in[7]),32);
    bufp->fullIData(oldp+11,(vlSelf->data_in[8]),32);
    bufp->fullIData(oldp+12,(vlSelf->data_in[9]),32);
    bufp->fullIData(oldp+13,(vlSelf->data_in[10]),32);
    bufp->fullIData(oldp+14,(vlSelf->data_in[11]),32);
    bufp->fullIData(oldp+15,(vlSelf->data_in[12]),32);
    bufp->fullIData(oldp+16,(vlSelf->data_in[13]),32);
    bufp->fullIData(oldp+17,(vlSelf->data_in[14]),32);
    bufp->fullIData(oldp+18,(vlSelf->data_in[15]),32);
    bufp->fullIData(oldp+19,(vlSelf->data_out[0]),32);
    bufp->fullIData(oldp+20,(vlSelf->data_out[1]),32);
    bufp->fullIData(oldp+21,(vlSelf->data_out[2]),32);
    bufp->fullIData(oldp+22,(vlSelf->data_out[3]),32);
    bufp->fullIData(oldp+23,(vlSelf->data_out[4]),32);
    bufp->fullIData(oldp+24,(vlSelf->data_out[5]),32);
    bufp->fullIData(oldp+25,(vlSelf->data_out[6]),32);
    bufp->fullIData(oldp+26,(vlSelf->data_out[7]),32);
    bufp->fullIData(oldp+27,(vlSelf->data_out[8]),32);
    bufp->fullIData(oldp+28,(vlSelf->data_out[9]),32);
    bufp->fullIData(oldp+29,(vlSelf->data_out[10]),32);
    bufp->fullIData(oldp+30,(vlSelf->data_out[11]),32);
    bufp->fullIData(oldp+31,(vlSelf->data_out[12]),32);
    bufp->fullIData(oldp+32,(vlSelf->data_out[13]),32);
    bufp->fullIData(oldp+33,(vlSelf->data_out[14]),32);
    bufp->fullIData(oldp+34,(vlSelf->data_out[15]),32);
    bufp->fullBit(oldp+35,(vlSelf->data_in_valid));
    bufp->fullBit(oldp+36,(vlSelf->data_in_ready));
    bufp->fullBit(oldp+37,(vlSelf->data_out_valid));
    bufp->fullBit(oldp+38,(vlSelf->data_out_ready));
    bufp->fullBit(oldp+39,(vlSelf->fixed_relu__DOT__rst));
    bufp->fullBit(oldp+40,(vlSelf->fixed_relu__DOT__clk));
    bufp->fullIData(oldp+41,(vlSelf->fixed_relu__DOT__data_in[0]),32);
    bufp->fullIData(oldp+42,(vlSelf->fixed_relu__DOT__data_in[1]),32);
    bufp->fullIData(oldp+43,(vlSelf->fixed_relu__DOT__data_in[2]),32);
    bufp->fullIData(oldp+44,(vlSelf->fixed_relu__DOT__data_in[3]),32);
    bufp->fullIData(oldp+45,(vlSelf->fixed_relu__DOT__data_in[4]),32);
    bufp->fullIData(oldp+46,(vlSelf->fixed_relu__DOT__data_in[5]),32);
    bufp->fullIData(oldp+47,(vlSelf->fixed_relu__DOT__data_in[6]),32);
    bufp->fullIData(oldp+48,(vlSelf->fixed_relu__DOT__data_in[7]),32);
    bufp->fullIData(oldp+49,(vlSelf->fixed_relu__DOT__data_in[8]),32);
    bufp->fullIData(oldp+50,(vlSelf->fixed_relu__DOT__data_in[9]),32);
    bufp->fullIData(oldp+51,(vlSelf->fixed_relu__DOT__data_in[10]),32);
    bufp->fullIData(oldp+52,(vlSelf->fixed_relu__DOT__data_in[11]),32);
    bufp->fullIData(oldp+53,(vlSelf->fixed_relu__DOT__data_in[12]),32);
    bufp->fullIData(oldp+54,(vlSelf->fixed_relu__DOT__data_in[13]),32);
    bufp->fullIData(oldp+55,(vlSelf->fixed_relu__DOT__data_in[14]),32);
    bufp->fullIData(oldp+56,(vlSelf->fixed_relu__DOT__data_in[15]),32);
    bufp->fullIData(oldp+57,(vlSelf->fixed_relu__DOT__data_out[0]),32);
    bufp->fullIData(oldp+58,(vlSelf->fixed_relu__DOT__data_out[1]),32);
    bufp->fullIData(oldp+59,(vlSelf->fixed_relu__DOT__data_out[2]),32);
    bufp->fullIData(oldp+60,(vlSelf->fixed_relu__DOT__data_out[3]),32);
    bufp->fullIData(oldp+61,(vlSelf->fixed_relu__DOT__data_out[4]),32);
    bufp->fullIData(oldp+62,(vlSelf->fixed_relu__DOT__data_out[5]),32);
    bufp->fullIData(oldp+63,(vlSelf->fixed_relu__DOT__data_out[6]),32);
    bufp->fullIData(oldp+64,(vlSelf->fixed_relu__DOT__data_out[7]),32);
    bufp->fullIData(oldp+65,(vlSelf->fixed_relu__DOT__data_out[8]),32);
    bufp->fullIData(oldp+66,(vlSelf->fixed_relu__DOT__data_out[9]),32);
    bufp->fullIData(oldp+67,(vlSelf->fixed_relu__DOT__data_out[10]),32);
    bufp->fullIData(oldp+68,(vlSelf->fixed_relu__DOT__data_out[11]),32);
    bufp->fullIData(oldp+69,(vlSelf->fixed_relu__DOT__data_out[12]),32);
    bufp->fullIData(oldp+70,(vlSelf->fixed_relu__DOT__data_out[13]),32);
    bufp->fullIData(oldp+71,(vlSelf->fixed_relu__DOT__data_out[14]),32);
    bufp->fullIData(oldp+72,(vlSelf->fixed_relu__DOT__data_out[15]),32);
    bufp->fullBit(oldp+73,(vlSelf->fixed_relu__DOT__data_in_valid));
    bufp->fullBit(oldp+74,(vlSelf->fixed_relu__DOT__data_in_ready));
    bufp->fullBit(oldp+75,(vlSelf->fixed_relu__DOT__data_out_valid));
    bufp->fullBit(oldp+76,(vlSelf->fixed_relu__DOT__data_out_ready));
}
