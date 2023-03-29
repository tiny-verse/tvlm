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
        virtual std::string name() const {
            return "generic pass";
        }
    };

    class LastPass : public Pass{

        void lastToOptimize(IL &il);
    public:
        void run (IL & il) override;
        std::string name() const override{
            return "Last Pass";
        }
    };
    class Optimizer {

        using IL = used_IL;
        using ILInstruction = ::tvlm::IL;
    public:
        Optimizer(){
            tiny::config.setDefaultIfMissing("-inlining", "0");
            inlining = std::stoul(tiny::config.get("-inlining"));
            tiny::config.setDefaultIfMissing("-optimization_cp", "1");
            constant = std::stoul(tiny::config.get("-optimization_cp"));
            tiny::config.setDefaultIfMissing("-optimization_dce", "1");
            deadCodeElimination = std::stoul(tiny::config.get("-optimization_dce"));
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
        bool deadCodeElimination = false;
    };



}