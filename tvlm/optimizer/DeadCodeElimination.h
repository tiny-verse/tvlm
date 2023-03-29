#pragma once

#include "Optimizer.h"
#include "tvlm/analysis/constantPropagation_analysis.h"

namespace tvlm{
    class DeadCodeElimination : public Pass {

        void optimizeDeadCodeElimination(BasicBlock *bb);

        void optimizeBasicBlock(BasicBlock* bb);


    public:
        void run(IL &il) override;
        std::string name() const override{
            return "DCE";
        }
    };
}