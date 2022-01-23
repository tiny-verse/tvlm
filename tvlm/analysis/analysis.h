#pragma once
#include "cfg.h"
namespace tvlm {
    template<class T>
    class Analysis {
    public:
        virtual T analyze() = 0;
    };

    template<typename T>
    class BackwardAnalysis : public Analysis<T> {
    protected:
        static ProgramCfg getCfg(Program * p){
            auto builder = std::make_unique<IntraProceduralCfgBuilder>();
            return builder->fromProgram(p);
        }
    public:
        virtual T analyze() = 0;
    };

    template<typename T>
    class ForwardAnalysis : public Analysis<T> {
    public:
        virtual T analyze() = 0;
    };
}
