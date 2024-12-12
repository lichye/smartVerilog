// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Primary model header
//
// This header should be included by all source files instantiating the design.
// The class here is then constructed to instantiate the design.
// See the Verilator manual for examples.

#ifndef VERILATED_VTOP_H_
#define VERILATED_VTOP_H_  // guard

#include "verilated.h"
#include "svdpi.h"

class Vtop__Syms;
class Vtop___024root;
class VerilatedVcdC;

// This class is the main interface to the Verilated model
class alignas(VL_CACHE_LINE_BYTES) Vtop VL_NOT_FINAL : public VerilatedModel {
  private:
    // Symbol table holding complete model state (owned by this class)
    Vtop__Syms* const vlSymsp;

  public:

    // PORTS
    // The application code writes and reads these signals to
    // propagate new values into/out from the Verilated model.
    VL_IN8(&N1,0,0);
    VL_IN8(&N5,0,0);
    VL_IN8(&N9,0,0);
    VL_IN8(&N13,0,0);
    VL_IN8(&N17,0,0);
    VL_IN8(&N21,0,0);
    VL_IN8(&N25,0,0);
    VL_IN8(&N29,0,0);
    VL_IN8(&N33,0,0);
    VL_IN8(&N37,0,0);
    VL_IN8(&N41,0,0);
    VL_IN8(&N45,0,0);
    VL_IN8(&N49,0,0);
    VL_IN8(&N53,0,0);
    VL_IN8(&N57,0,0);
    VL_IN8(&N61,0,0);
    VL_IN8(&N65,0,0);
    VL_IN8(&N69,0,0);
    VL_IN8(&N73,0,0);
    VL_IN8(&N77,0,0);
    VL_IN8(&N81,0,0);
    VL_IN8(&N85,0,0);
    VL_IN8(&N89,0,0);
    VL_IN8(&N93,0,0);
    VL_IN8(&N97,0,0);
    VL_IN8(&N101,0,0);
    VL_IN8(&N105,0,0);
    VL_IN8(&N109,0,0);
    VL_IN8(&N113,0,0);
    VL_IN8(&N117,0,0);
    VL_IN8(&N121,0,0);
    VL_IN8(&N125,0,0);
    VL_IN8(&N129,0,0);
    VL_IN8(&N130,0,0);
    VL_IN8(&N131,0,0);
    VL_IN8(&N132,0,0);
    VL_IN8(&N133,0,0);
    VL_IN8(&N134,0,0);
    VL_IN8(&N135,0,0);
    VL_IN8(&N136,0,0);
    VL_IN8(&N137,0,0);
    VL_OUT8(&N724,0,0);
    VL_OUT8(&N725,0,0);
    VL_OUT8(&N726,0,0);
    VL_OUT8(&N727,0,0);
    VL_OUT8(&N728,0,0);
    VL_OUT8(&N729,0,0);
    VL_OUT8(&N730,0,0);
    VL_OUT8(&N731,0,0);
    VL_OUT8(&N732,0,0);
    VL_OUT8(&N733,0,0);
    VL_OUT8(&N734,0,0);
    VL_OUT8(&N735,0,0);
    VL_OUT8(&N736,0,0);
    VL_OUT8(&N737,0,0);
    VL_OUT8(&N738,0,0);
    VL_OUT8(&N739,0,0);
    VL_OUT8(&N740,0,0);
    VL_OUT8(&N741,0,0);
    VL_OUT8(&N742,0,0);
    VL_OUT8(&N743,0,0);
    VL_OUT8(&N744,0,0);
    VL_OUT8(&N745,0,0);
    VL_OUT8(&N746,0,0);
    VL_OUT8(&N747,0,0);
    VL_OUT8(&N748,0,0);
    VL_OUT8(&N749,0,0);
    VL_OUT8(&N750,0,0);
    VL_OUT8(&N751,0,0);
    VL_OUT8(&N752,0,0);
    VL_OUT8(&N753,0,0);
    VL_OUT8(&N754,0,0);
    VL_OUT8(&N755,0,0);

    // CELLS
    // Public to allow access to /* verilator public */ items.
    // Otherwise the application code can consider these internals.

    // Root instance pointer to allow access to model internals,
    // including inlined /* verilator public_flat_* */ items.
    Vtop___024root* const rootp;

    // CONSTRUCTORS
    /// Construct the model; called by application code
    /// If contextp is null, then the model will use the default global context
    /// If name is "", then makes a wrapper with a
    /// single model invisible with respect to DPI scope names.
    explicit Vtop(VerilatedContext* contextp, const char* name = "TOP");
    explicit Vtop(const char* name = "TOP");
    /// Destroy the model; called (often implicitly) by application code
    virtual ~Vtop();
  private:
    VL_UNCOPYABLE(Vtop);  ///< Copying not allowed

  public:
    // API METHODS
    /// Evaluate the model.  Application must call when inputs change.
    void eval() { eval_step(); }
    /// Evaluate when calling multiple units/models per time step.
    void eval_step();
    /// Evaluate at end of a timestep for tracing, when using eval_step().
    /// Application must call after all eval() and before time changes.
    void eval_end_step() {}
    /// Simulation complete, run final blocks.  Application must call on completion.
    void final();
    /// Are there scheduled events to handle?
    bool eventsPending();
    /// Returns time at next time slot. Aborts if !eventsPending()
    uint64_t nextTimeSlot();
    /// Trace signals in the model; called by application code
    void trace(VerilatedVcdC* tfp, int levels, int options = 0);
    /// Retrieve name of this model instance (as passed to constructor).
    const char* name() const;

    // Abstract methods from VerilatedModel
    const char* hierName() const override final;
    const char* modelName() const override final;
    unsigned threads() const override final;
    /// Prepare for cloning the model at the process level (e.g. fork in Linux)
    /// Release necessary resources. Called before cloning.
    void prepareClone() const;
    /// Re-init after cloning the model at the process level (e.g. fork in Linux)
    /// Re-allocate necessary resources. Called after cloning.
    void atClone() const;
    std::unique_ptr<VerilatedTraceConfig> traceConfig() const override final;
};

#endif  // guard
