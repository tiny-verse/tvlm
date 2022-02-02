#include "RegisterAllocator.h"

tvlm::RegisterAllocator::RegisterAllocator() {
        alloc_regs_.resize(tiny::t86::Cpu::Config::instance().registerCnt());
        alloc_fregs_.resize(tiny::t86::Cpu::Config::instance().floatRegisterCnt());
    }

