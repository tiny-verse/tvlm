#include "RegisterAllocator.h"

namespace tvlm {

    RegisterAllocator::RegisterAllocator(ProgramBuilderOLD * pb) : pb_(pb){
        alloc_regs_.resize(tiny::t86::Cpu::Config::instance().registerCnt());
        alloc_fregs_.resize(tiny::t86::Cpu::Config::instance().floatRegisterCnt());
    }

}

