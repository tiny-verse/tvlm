#pragma once
#include "cfg.h"

namespace tvlm {


    template<class T>
    class Analysis {
    public:
        virtual T  analyze() = 0;
        virtual ~Analysis() = default;
    };

    template<typename T, typename I>
    class BackwardAnalysis : public Analysis<T> {
    protected:
        static ProgramCfg<I> * getCfg(Program * p){
            auto builder = std::make_unique<IntraProceduralCfgBuilder<I>>();
            return builder->fromProgram(p);
        }
    public:
        virtual ~BackwardAnalysis() = default;
        virtual T analyze() = 0;
    };

    template<typename T, typename I>
    class ForwardAnalysis : public Analysis<T> {
    public:
        virtual ~ForwardAnalysis() = default;
        virtual T analyze() = 0;
    protected:
        static ProgramCfg<I> * getCfg(Program * p){
            auto builder = std::make_unique<IntraProceduralCfgBuilder<I>>();
            return builder->fromProgram(p);
        }
    };
}
