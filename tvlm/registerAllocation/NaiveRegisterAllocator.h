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

        Register getReg(const Instruction *ins) override;

        FRegister getFloatReg(const Instruction *ins) override;

        void clearInt(const Instruction *ins) override;

        void clearFloat(const Instruction *ins) override;

        void spillCallReg() override;

        void clearAllReg() override;

        void spillAllReg() override;

        void registerPhi(Phi *phi) override;

        void clearIntRegister(const Register &reg) override;

        void prepareReturnValue(size_t size,const Instruction * ret)override;

        void makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins) override;

        void allocateStructArg(Type * type,const Instruction * ins) override;

        void resetAllocSize() override;

        void correctStackAlloc(size_t patch) override;

    protected:
        Register getFreeIntRegister() override;

        FRegister getFreeFloatRegister() override;

        Register getIntRegister(const Instruction *ins) override;

        FRegister getFloatRegister(const Instruction *ins) override;

        int counter = 1;
        int fcounter = 1;
        const int reserve_upbound = 1;
        size_t functionLocalAllocSize = 0;


        std::unordered_map<const Instruction *, int64_t> spilled_; //ins, mem_offset //address descriptor
        std::pair<bool, int> findInIntRegs(const Instruction *ins);
        std::pair<bool, int> findInFloatRegs(const Instruction *ins);

        bool spillIntReg(const Register & reg, const Instruction * ins);


        bool spillFloatReg(const FRegister & reg, const Instruction * ins);


        void copyStruct(Register aRegister, Type *pType, Register aRegister1, const Instruction * ins);

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