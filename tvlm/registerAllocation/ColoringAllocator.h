#pragma once

#include "RegisterAllocator.h"
#include "t86/instruction.h"
#include "t86/t86_target"
#include "t86/program.h"
#include "t86/program/programbuilder.h"
#include "tvlm/InstructionSelection/ProgramBuilder.h"

#include "tvlm/analysis/liveness_analysis.h"


/**
 * Preparation:
 *      Instruction Selection done - TInstruction known
 *          - Nontrivial //TODO
 *      Naively assigned Registers
 *      Liveness analysis done //TODO
 *          - Cfg construction //TODO
 * Work:
 *      Rework assigned registers by idea of graph coloring
 * */
namespace tvlm{

    class ColorInfo{

    };


    class ColoringAllocator : public RegisterAllocator{
    public:
        void ReassignRegisters(ILBuilder & ilb /*or ProgramBuilder and res of analysis*/){
            auto prog = ilb.finish();
            auto la = new LivenessAnalysis<ColorInfo>(&prog); // Integrate ILBuilder and ProgramBuilder
            analysisResult_ = la->analyze();

        }
        void ReassignRegisters(Program * prog /*or ProgramBuilder and res of analysis*/){
            auto la = new LivenessAnalysis<ColorInfo>( prog); // Integrate ILBuilder and ProgramBuilder
            analysisResult_ = la->analyze();

        }

        Register getReg(Instruction *ins) override;

        FRegister getFloatReg(Instruction *ins) override;

        void clearInt(Instruction *ins) override;

        void clearFloat(Instruction *ins) override;

        void spillCallReg() override;

        void clearAllReg() override;

        void spillAllReg() override;

        void prepareReturnValue(size_t size, Instruction * ret) override;

        void makeLocalAllocation(size_t size, const Register &reg, Instruction * ins) override;

        void allocateStructArg(Type *type, Instruction *ins) override;

        void resetAllocSize() override;

        void correctStackAlloc(size_t patch) override;

    protected:
        Register getIntRegister(Instruction *ins) override;

        FRegister getFloatRegister(Instruction *ins) override;

        Register getFreeIntRegister() override;

        FRegister getFreeFloatRegister() override;

        MAP<const CfgNode<ColorInfo> * , std::unordered_set<IL*>> analysisResult_;
    };
}
