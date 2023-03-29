#pragma once

#include "Optimizer.h"
#include "tvlm/il/il_builder.h"

namespace tvlm{

    class FunctionInliner : public Pass {

        void functionInlining(IL & il);

        bool eligibleForInlining(Function *fnc);


        void copyInstructions(std::vector<std::unique_ptr < Instruction>>::iterator beginSource,
                              const std::vector<std::unique_ptr < Instruction>>::iterator &endSource,
                              std::insert_iterator<std::vector < std::unique_ptr < Instruction>>>  beginDestination,
                              std::unordered_map<Instruction *, Instruction *> &swapInstr,
                              const std::vector <std::function<std::unique_ptr<Instruction>(std::unique_ptr < Instruction > &)>> &lambdaChange
        );

        void copyInstructions(const std::vector<std::unique_ptr < Instruction>>::iterator &beginSource,
                              const std::vector<std::unique_ptr < Instruction>>::iterator &endSource,
                              const std::insert_iterator <std::vector<std::unique_ptr < Instruction>>> & beginDestination,
                              const std::vector <std::function<std::unique_ptr<Instruction>(std::unique_ptr < Instruction > &)>> &lambdaChange
        );

    public:
        virtual ~FunctionInliner(){}
        void run(IL & il) override{
            functionInlining(il);
        }
        std::string name() const override{
            return "Fnc Inliner";
        }
    };
}
