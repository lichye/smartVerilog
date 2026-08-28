// smart.out: the single-block worker, kept as its own binary for debugging.
// The pipeline does not use it — it re-execs itself with `--block` — but a
// block is much easier to run by hand this way.
//
//   smart.out <workdir> <top> <result_file> <variables_file> <core_id>
//             <latency> <config.json>
//
// Must be run with the working directory set to <workdir>.

#include "BlockRunner.h"

int main(int argc, char* argv[]) {
    return smart::pipeline::runSmartBlock(argc, argv);
}
