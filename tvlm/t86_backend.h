#pragma once

#include <unordered_set>
#include <utility>
#include "tvlm/tvlm/il/il.h"
#include "tvlm/tvlm/il/il_builder.h"
#include "t86/program/programbuilder.h"

namespace tvlm {

    using Opcode = tvlm::Instruction::Opcode;

    class t86_Backend{
    public:
        static int MemoryCellSize;

        static  void tests();

        using IL = tvlm::Program;
        using TargetProgram = tiny::t86::Program;
        TargetProgram compileToTarget( IL && il);
    };
} // namespace tvlm

