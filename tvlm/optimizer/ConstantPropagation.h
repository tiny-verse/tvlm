#pragma once

#include "Optimizer.h"

namespace tvlm{
    class ConstantPropagation : public Pass {

        void optimizeConstantPropagation(BasicBlock *bb);

        void optimizeBasicBlock(BasicBlock* bb);

        void optimizeStrengthReduction(BasicBlock *bb);

    public:
        void run(IL &il);

        int64_t resolveBinOperator(BinOp *bin, int64_t lhs, int64_t rhs);

        static bool isPowerOfTwo(int num);

    };
}