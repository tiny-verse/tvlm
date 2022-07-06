#pragma once

#include "RegisterAllocator.h"
#include "t86/instruction.h"
#include "t86/t86_target"
#include "t86/program.h"
#include "../InstructionSelection/ProgramBuilder.h"

namespace tvlm{

    using Register = tvlm::Register;
    class NaiveRegisterAllocator :public RegisterAllocator{

    public:
        NaiveRegisterAllocator(ProgramBuilder * pb):
                RegisterAllocator(pb){}

        Register getReg(Instruction *ins) override;

        FRegister getFloatReg(Instruction *ins) override;

        void clearInt(Instruction *ins) override;

        void clearFloat(Instruction *ins) override;

        void spillCallReg() override;

        void clearAllReg() override;

        void spillAllReg() override;

        void registerPhi(Phi *phi) override;

        void clearIntRegister(const Register &reg) override;

        void prepareReturnValue(size_t size, Instruction * ret)override;

        void makeLocalAllocation(size_t size, const Register &reg, Instruction * ins) override;

        void allocateStructArg(Type * type, Instruction * ins) override;

        void resetAllocSize() override;

        void correctStackAlloc(size_t patch) override;

    protected:
        Register getFreeIntRegister() override;

        FRegister getFreeFloatRegister() override;

        Register getIntRegister(Instruction *ins) override;

        FRegister getFloatRegister(Instruction *ins) override;

        int counter = 1;
        int fcounter = 1;
        const int reserve_upbound = 1;
        size_t functionLocalAllocSize = 0;


        std::unordered_map<const Instruction *, int64_t> spilled_; //ins, mem_offset
        std::pair<bool, int> findInIntRegs(Instruction *ins);
        std::pair<bool, int> findInFloatRegs(Instruction *ins);

        bool spillIntReg(const Register & reg, Instruction * ins);


        bool spillFloatReg(const FRegister & reg, Instruction * ins);


        void copyStruct(Register aRegister, Type *pType, Register aRegister1, Instruction * ins);

        template<typename T>
        void replace(size_t label,const T & instruction){
            pb_->replace(label, instruction);
//            if(label >= pb_->currentLabel()){
//                //ERROR
//                std::cerr << "replace unable to continue" << std::endl;
//                return;
//            }
//            auto p = pb_->program();
//            auto instrs = p.moveInstructions();
//            delete instrs[label];
//            instrs[label] = new T(instruction);
//
//            auto resProg = tiny::t86::Program(instrs, p.data());
//            *pb_ = ProgramBuilder(std::move(resProg));
        }
    };


}