#pragma once

#include "t86/program/helpers.h"
#include "tvlm/tvlm/il/il.h"
#include "tvlm/InstructionSelection/ProgramBuilder.h"

namespace tvlm{
    using Register = tiny::t86::Register;
    using FRegister = tiny::t86::FloatRegister;
    using TInstruction = tiny::t86::Instruction;
    using ILInstruction = ::tvlm::Instruction;

    /*abstract*/ class RegisterAllocator {

    public:
        virtual ~RegisterAllocator() = default;
        explicit RegisterAllocator(ProgramBuilder * pb);

        /**
         * getReg : Code generator uses getReg function to determine the status of available registers
         *           and the location of name values. getReg works as follows:\n
         *
         * - If variable Y is already in register R, it uses that register.\n
         * - Else if some register R is available, it uses that register.\n
         * - Else if both the above options are not possible, it chooses a register that requires minimal number of load and store instructions.
         * */
        virtual Register getReg(ILInstruction * ins) = 0;
        virtual FRegister getFloatReg(ILInstruction * ins) = 0;

        virtual void clearInt(ILInstruction * ins) = 0;
        virtual void clearFloat(ILInstruction * ins) = 0;
        virtual void spillCallReg() = 0;
        virtual void clearAllReg() = 0;


        Register fillIntRegister(){
            auto reg = getFreeIntRegister();
            tmpIntRegs_.emplace(reg);
            return reg;
        }

        FRegister fillFloatRegister(){
            auto reg = getFreeFloatRegister();
            tmpFloatRegs_.emplace(reg);
            return reg;
        }

        virtual void clearIntRegister(const Register & reg ){
            tmpIntRegs_.erase(reg);
            alloc_regs_[reg.index()] = nullptr;
        }

        void replaceInt(ILInstruction * from, ILInstruction * to ){
            alloc_regs_[getIntRegister(from).index()] = to;
        }
        void replaceFloat(ILInstruction * from, ILInstruction * to ){
            alloc_fregs_[getFloatRegister(from).index()] = to;
        }
        virtual void spillAllReg()  = 0;

        virtual void registerPhi(Phi * phi) {
            for(auto & content : phi->contents() ){
                auto f = std::find(alloc_regs_.begin(), alloc_regs_.end(), content.second);
                if(f != alloc_regs_.end()){
                    *f = phi;
                    break;
                }
            }
        }


        virtual void prepareReturnValue(size_t size, ILInstruction * ret) = 0;
        virtual void makeLocalAllocation(size_t size, const Register & reg, ILInstruction * ins) = 0;

        virtual void allocateStructArg(Type * type, ILInstruction * ins) = 0;

        virtual void resetAllocSize() = 0;

        virtual void correctStackAlloc(size_t patch ) = 0;

    protected:
        virtual Register getIntRegister(ILInstruction * ins) = 0;
        virtual FRegister getFloatRegister(ILInstruction * ins) = 0;

        virtual Register getFreeIntRegister() = 0; //care not to take tmpAllocated
        virtual FRegister getFreeFloatRegister() = 0; //care not to take tmpAllocated


        std::vector< ILInstruction *> alloc_regs_;
        std::vector< ILInstruction *> alloc_fregs_;

        std::set<Register> tmpIntRegs_;
        std::set<FRegister> tmpFloatRegs_;

        ProgramBuilder * pb_;


    };
}
