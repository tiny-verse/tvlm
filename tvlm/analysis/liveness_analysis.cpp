#include "liveness_analysis.h"

tvlm::LivenessAnalysis tvlm::LivenessAnalysis::create(Program *p){
    auto analysis = InstructionAnalysis(p);
    Declarations decls = analysis.analyze();
    auto builder = new IntraProceduralCfgBuilder;
    auto cfg = builder->fromProgram(p);

    return LivenessAnalysis(cfg, decls);
}

