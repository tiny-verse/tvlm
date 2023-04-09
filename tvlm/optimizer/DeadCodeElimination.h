#pragma once

#include "Optimizer.h"
#include "tvlm/analysis/constantPropagation_analysis.h"

namespace tvlm{
    class DeadCodeElimination : public Pass {

        void optimizeBasicBlock(BasicBlock* bb);

        void optimizeDeadCodeElimination(BasicBlock *bb);


    public:
        void run(IL &il) override;
        std::string name() const override{
            return "DCE";
        }

        bool unusedFunction(IL &program, const Symbol &symbol);
    };
}