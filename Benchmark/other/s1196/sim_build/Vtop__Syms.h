// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VTOP__SYMS_H_
#define VERILATED_VTOP__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "Vtop.h"

// INCLUDE MODULE CLASSES
#include "Vtop___024root.h"

// DPI TYPES for DPI Export callbacks (Internal use)

// SYMS CLASS (contains all model state)
class alignas(VL_CACHE_LINE_BYTES)Vtop__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    Vtop* const __Vm_modelp;
    bool __Vm_activity = false;  ///< Used by trace routines to determine change occurred
    uint32_t __Vm_baseCode = 0;  ///< Used by trace routines when tracing multiple models
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    Vtop___024root                 TOP;

    // SCOPE NAMES
    VerilatedScope __Vscope_TOP;
    VerilatedScope __Vscope_s1196;
    VerilatedScope __Vscope_s1196__DFF_0;
    VerilatedScope __Vscope_s1196__DFF_1;
    VerilatedScope __Vscope_s1196__DFF_10;
    VerilatedScope __Vscope_s1196__DFF_11;
    VerilatedScope __Vscope_s1196__DFF_12;
    VerilatedScope __Vscope_s1196__DFF_13;
    VerilatedScope __Vscope_s1196__DFF_14;
    VerilatedScope __Vscope_s1196__DFF_15;
    VerilatedScope __Vscope_s1196__DFF_16;
    VerilatedScope __Vscope_s1196__DFF_17;
    VerilatedScope __Vscope_s1196__DFF_2;
    VerilatedScope __Vscope_s1196__DFF_3;
    VerilatedScope __Vscope_s1196__DFF_4;
    VerilatedScope __Vscope_s1196__DFF_5;
    VerilatedScope __Vscope_s1196__DFF_6;
    VerilatedScope __Vscope_s1196__DFF_7;
    VerilatedScope __Vscope_s1196__DFF_8;
    VerilatedScope __Vscope_s1196__DFF_9;

    // SCOPE HIERARCHY
    VerilatedHierarchy __Vhier;

    // CONSTRUCTORS
    Vtop__Syms(VerilatedContext* contextp, const char* namep, Vtop* modelp);
    ~Vtop__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
};

#endif  // guard
