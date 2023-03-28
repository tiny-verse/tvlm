#pragma once

#include "tvlm/il/il_builder.h"
#include "tvlm/il/il.h"
#include "common/config.h"
#include <memory>

namespace tvlm {
#define used_IL ::tvlm::Program
    class Pass : public ILFriend{
    protected:
        using IL = used_IL;
    public:
        virtual ~Pass(){};
        virtual void run(IL & il) = 0;
    };

    class LastPass : public Pass{

        void lastToOptimize(IL &il);
    public:
        void run (IL & il) override;
    };
    class Optimizer {

        using IL = used_IL;
        using ILInstruction = ::tvlm::IL;
    public:
        Optimizer(){
            tiny::config.setDefaultIfMissing("-inlining", "0");
            inlining = std::stoul(tiny::config.get("-inlining"));
            tiny::config.setDefaultIfMissing("-constant_propagation", "1");
            constant = std::stoul(tiny::config.get("-constant_propagation"));
            initialize_passes();
        }

        void initialize_passes();
        void optimize(IL &ir);



        void functionInlining(IL &ir);


        static bool eligibleForInlining(Function *fnc);


        void addPass(Pass * pass){
            passes_.push_back(std::unique_ptr<Pass>(pass));
        }
    private:
        std::vector<std::unique_ptr<Pass>> passes_;
        bool inlining = false;
        bool constant = false;
    };



}