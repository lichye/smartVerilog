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
    VL_IN8(&N4,0,0);
    VL_IN8(&N8,0,0);
    VL_IN8(&N11,0,0);
    VL_IN8(&N14,0,0);
    VL_IN8(&N17,0,0);
    VL_IN8(&N21,0,0);
    VL_IN8(&N24,0,0);
    VL_IN8(&N27,0,0);
    VL_IN8(&N30,0,0);
    VL_IN8(&N34,0,0);
    VL_IN8(&N37,0,0);
    VL_IN8(&N40,0,0);
    VL_IN8(&N43,0,0);
    VL_IN8(&N47,0,0);
    VL_IN8(&N50,0,0);
    VL_IN8(&N53,0,0);
    VL_IN8(&N56,0,0);
    VL_IN8(&N60,0,0);
    VL_IN8(&N63,0,0);
    VL_IN8(&N66,0,0);
    VL_IN8(&N69,0,0);
    VL_IN8(&N73,0,0);
    VL_IN8(&N76,0,0);
    VL_IN8(&N79,0,0);
    VL_IN8(&N82,0,0);
    VL_IN8(&N86,0,0);
    VL_IN8(&N89,0,0);
    VL_IN8(&N92,0,0);
    VL_IN8(&N95,0,0);
    VL_IN8(&N99,0,0);
    VL_IN8(&N102,0,0);
    VL_IN8(&N105,0,0);
    VL_IN8(&N108,0,0);
    VL_IN8(&N112,0,0);
    VL_IN8(&N115,0,0);
    VL_OUT8(&N223,0,0);
    VL_OUT8(&N329,0,0);
    VL_OUT8(&N370,0,0);
    VL_OUT8(&N421,0,0);
    VL_OUT8(&N430,0,0);
    VL_OUT8(&N431,0,0);
    VL_OUT8(&N432,0,0);

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
