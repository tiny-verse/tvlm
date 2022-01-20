#pragma once

#include "t86/program/helpers.h"
#include "tvlm/il.h"

namespace tvlm{
    using Register = tiny::t86::Register;
    using FRegister = tiny::t86::FloatRegister;

    /*abstract*/ class RegisterAllocator {
    public:
            virtual Register fillIntRegister(Instruction * ins) = 0;
            virtual FRegister fillFloatRegister(Instruction * ins) = 0;
            virtual Register getIntRegister(Instruction * ins) = 0;
            virtual FRegister getFloatRegister(Instruction * ins) = 0;

            virtual void clearInt(Instruction * ins) = 0;
            virtual void clearAllIntReg() = 0;
            virtual void clearAllFloatReg() = 0;
            virtual void clearAllReg() {
                clearAllIntReg();
                return clearAllFloatReg();
            }
            virtual void spillAllReg()  = 0;
            std::map<Register, Instruction *> alloc_regs_;
            std::map<Register, Instruction *> alloc_fregs_;

    };
}
