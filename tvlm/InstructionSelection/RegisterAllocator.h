#pragma once

#include "t86/program/helpers.h"
#include "tvlm/tvlm/il/il.h"

namespace tvlm{
    using Register = tiny::t86::Register;
    using FRegister = tiny::t86::FloatRegister;

    /*abstract*/ class RegisterAllocator {
    public:
            virtual Register fillIntRegister(Instruction * ins) = 0;
            virtual FRegister fillFloatRegister(Instruction * ins) = 0;

            virtual void clearInt(Instruction * ins) = 0;
            virtual void clearFloat(Instruction * ins) = 0;
            virtual void clearAllIntReg() = 0;
            virtual void clearAllFloatReg() = 0;
            virtual void clearAllReg() {
                clearAllIntReg();
                return clearAllFloatReg();
            }

            void replaceInt(Instruction * from, Instruction * to ){
                alloc_regs_[getIntRegister(from).index()] = to;
            }
            void replaceFloat(Instruction * from, Instruction * to ){
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


    protected:
            virtual Register getIntRegister(Instruction * ins) = 0;
            virtual FRegister getFloatRegister(Instruction * ins) = 0;

            std::vector< Instruction *> alloc_regs_;
            std::vector< Instruction *> alloc_fregs_;

    };
}
