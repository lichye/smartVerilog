// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "Vtop__pch.h"
#include "verilated_vcd_c.h"

//============================================================
// Constructors

Vtop::Vtop(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new Vtop__Syms(contextp(), _vcname__, this)}
    , N1{vlSymsp->TOP.N1}
    , N5{vlSymsp->TOP.N5}
    , N9{vlSymsp->TOP.N9}
    , N13{vlSymsp->TOP.N13}
    , N17{vlSymsp->TOP.N17}
    , N21{vlSymsp->TOP.N21}
    , N25{vlSymsp->TOP.N25}
    , N29{vlSymsp->TOP.N29}
    , N33{vlSymsp->TOP.N33}
    , N37{vlSymsp->TOP.N37}
    , N41{vlSymsp->TOP.N41}
    , N45{vlSymsp->TOP.N45}
    , N49{vlSymsp->TOP.N49}
    , N53{vlSymsp->TOP.N53}
    , N57{vlSymsp->TOP.N57}
    , N61{vlSymsp->TOP.N61}
    , N65{vlSymsp->TOP.N65}
    , N69{vlSymsp->TOP.N69}
    , N73{vlSymsp->TOP.N73}
    , N77{vlSymsp->TOP.N77}
    , N81{vlSymsp->TOP.N81}
    , N85{vlSymsp->TOP.N85}
    , N89{vlSymsp->TOP.N89}
    , N93{vlSymsp->TOP.N93}
    , N97{vlSymsp->TOP.N97}
    , N101{vlSymsp->TOP.N101}
    , N105{vlSymsp->TOP.N105}
    , N109{vlSymsp->TOP.N109}
    , N113{vlSymsp->TOP.N113}
    , N117{vlSymsp->TOP.N117}
    , N121{vlSymsp->TOP.N121}
    , N125{vlSymsp->TOP.N125}
    , N129{vlSymsp->TOP.N129}
    , N130{vlSymsp->TOP.N130}
    , N131{vlSymsp->TOP.N131}
    , N132{vlSymsp->TOP.N132}
    , N133{vlSymsp->TOP.N133}
    , N134{vlSymsp->TOP.N134}
    , N135{vlSymsp->TOP.N135}
    , N136{vlSymsp->TOP.N136}
    , N137{vlSymsp->TOP.N137}
    , N724{vlSymsp->TOP.N724}
    , N725{vlSymsp->TOP.N725}
    , N726{vlSymsp->TOP.N726}
    , N727{vlSymsp->TOP.N727}
    , N728{vlSymsp->TOP.N728}
    , N729{vlSymsp->TOP.N729}
    , N730{vlSymsp->TOP.N730}
    , N731{vlSymsp->TOP.N731}
    , N732{vlSymsp->TOP.N732}
    , N733{vlSymsp->TOP.N733}
    , N734{vlSymsp->TOP.N734}
    , N735{vlSymsp->TOP.N735}
    , N736{vlSymsp->TOP.N736}
    , N737{vlSymsp->TOP.N737}
    , N738{vlSymsp->TOP.N738}
    , N739{vlSymsp->TOP.N739}
    , N740{vlSymsp->TOP.N740}
    , N741{vlSymsp->TOP.N741}
    , N742{vlSymsp->TOP.N742}
    , N743{vlSymsp->TOP.N743}
    , N744{vlSymsp->TOP.N744}
    , N745{vlSymsp->TOP.N745}
    , N746{vlSymsp->TOP.N746}
    , N747{vlSymsp->TOP.N747}
    , N748{vlSymsp->TOP.N748}
    , N749{vlSymsp->TOP.N749}
    , N750{vlSymsp->TOP.N750}
    , N751{vlSymsp->TOP.N751}
    , N752{vlSymsp->TOP.N752}
    , N753{vlSymsp->TOP.N753}
    , N754{vlSymsp->TOP.N754}
    , N755{vlSymsp->TOP.N755}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

Vtop::Vtop(const char* _vcname__)
    : Vtop(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

Vtop::~Vtop() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void Vtop___024root___eval_debug_assertions(Vtop___024root* vlSelf);
#endif  // VL_DEBUG
void Vtop___024root___eval_static(Vtop___024root* vlSelf);
void Vtop___024root___eval_initial(Vtop___024root* vlSelf);
void Vtop___024root___eval_settle(Vtop___024root* vlSelf);
void Vtop___024root___eval(Vtop___024root* vlSelf);

void Vtop::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vtop::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    Vtop___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_activity = true;
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        Vtop___024root___eval_static(&(vlSymsp->TOP));
        Vtop___024root___eval_initial(&(vlSymsp->TOP));
        Vtop___024root___eval_settle(&(vlSymsp->TOP));
    }
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    Vtop___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool Vtop::eventsPending() { return false; }

uint64_t Vtop::nextTimeSlot() {
    VL_FATAL_MT(__FILE__, __LINE__, "", "%Error: No delays in the design");
    return 0;
}

//============================================================
// Utilities

const char* Vtop::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void Vtop___024root___eval_final(Vtop___024root* vlSelf);

VL_ATTR_COLD void Vtop::final() {
    Vtop___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* Vtop::hierName() const { return vlSymsp->name(); }
const char* Vtop::modelName() const { return "Vtop"; }
unsigned Vtop::threads() const { return 1; }
void Vtop::prepareClone() const { contextp()->prepareClone(); }
void Vtop::atClone() const {
    contextp()->threadPoolpOnClone();
}
std::unique_ptr<VerilatedTraceConfig> Vtop::traceConfig() const {
    return std::unique_ptr<VerilatedTraceConfig>{new VerilatedTraceConfig{false, false, false}};
};

//============================================================
// Trace configuration

void Vtop___024root__trace_decl_types(VerilatedVcd* tracep);

void Vtop___024root__trace_init_top(Vtop___024root* vlSelf, VerilatedVcd* tracep);

VL_ATTR_COLD static void trace_init(void* voidSelf, VerilatedVcd* tracep, uint32_t code) {
    // Callback from tracep->open()
    Vtop___024root* const __restrict vlSelf VL_ATTR_UNUSED = static_cast<Vtop___024root*>(voidSelf);
    Vtop__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    if (!vlSymsp->_vm_contextp__->calcUnusedSigs()) {
        VL_FATAL_MT(__FILE__, __LINE__, __FILE__,
            "Turning on wave traces requires Verilated::traceEverOn(true) call before time 0.");
    }
    vlSymsp->__Vm_baseCode = code;
    tracep->pushPrefix(std::string{vlSymsp->name()}, VerilatedTracePrefixType::SCOPE_MODULE);
    Vtop___024root__trace_decl_types(tracep);
    Vtop___024root__trace_init_top(vlSelf, tracep);
    tracep->popPrefix();
}

VL_ATTR_COLD void Vtop___024root__trace_register(Vtop___024root* vlSelf, VerilatedVcd* tracep);

VL_ATTR_COLD void Vtop::trace(VerilatedVcdC* tfp, int levels, int options) {
    if (tfp->isOpen()) {
        vl_fatal(__FILE__, __LINE__, __FILE__,"'Vtop::trace()' shall not be called after 'VerilatedVcdC::open()'.");
    }
    if (false && levels && options) {}  // Prevent unused
    tfp->spTrace()->addModel(this);
    tfp->spTrace()->addInitCb(&trace_init, &(vlSymsp->TOP));
    Vtop___024root__trace_register(&(vlSymsp->TOP), tfp->spTrace());
}
