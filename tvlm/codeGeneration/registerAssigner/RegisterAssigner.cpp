#include "RegisterAssigner.h"
#include "t86/program/helpers.h"
#include "tvlm/tvlm/il/il.h"

namespace tvlm{


    RegisterAssigner::RegisterAssigner(ProgramBuilder *pb) {

    }


    void RegisterAssigner::makeLocalAllocation(size_t size, const Register &reg, const ILInstruction *ins) {
        functionLocalAllocSize += size;
        // already allocated, now just find addr for this allocation
        pb_->add(tiny::t86::MOV(reg, tiny::t86::Bp()), ins);
        pb_->add(tiny::t86::SUB(reg, (int64_t) functionLocalAllocSize), ins);
    }

}