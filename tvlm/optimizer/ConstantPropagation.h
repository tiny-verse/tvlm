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
        double resolveBinOperator(BinOp *bin, double lhs, double rhs);
        int64_t resolveUnOperator(UnOp *bin, int64_t lhs);
        double resolveUnOperator(UnOp *bin, double lhs);

        static bool isPowerOfTwo(int64_t num);

    };
}