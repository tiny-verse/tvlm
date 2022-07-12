#pragma once


#include "tvlm/tvlm/il/il.h"
#include "t86/program/label.h"
#include "t86/program/programbuilder.h"
#include "tvlm/tvlm/codeGeneration/registerAllocation/RegisterAllocator.h"
#include "tvlm/tvlm/codeGeneration/registerAllocation/NaiveRegisterAllocator.h"



namespace tvlm {

    class TilingIS {
        static tiny::t86::Program translate(ILBuilder &ilb);

    };

}