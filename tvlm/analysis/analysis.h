#pragma once
#include "tvlm_backend"


namespace tvlm {
    using Instruction = ::tvlm::Instruction;

    template<class T>
    class Analysis {

        virtual T analyze() = 0;
    };

    template<typename T>
    class BackwardAnalysis : public Analysis<T> {

        virtual T analyze() = 0;
    };

    template<typename T>
    class ForwardAnalysis : public Analysis<T> {

        virtual T analyze() = 0;
    };
}
