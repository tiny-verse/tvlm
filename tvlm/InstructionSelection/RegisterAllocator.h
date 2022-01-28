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

            Register fillTmpIntRegister(){
                auto reg = getFreeIntRegister();
                tmpIntRegs_.emplace(reg);
                return reg;
            }
            FRegister fillTmpFloatRegister(){
                auto reg = getFreeFloatRegister();
                tmpFloatRegs_.emplace(reg);
                return reg;
            }

            virtual void clearTmpIntRegister(const Register & reg ){
                tmpIntRegs_.erase(reg);
                alloc_regs_[reg.index()] = nullptr;
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

            virtual Register getFreeIntRegister() = 0; //care not to take tmpAllocated
            virtual FRegister getFreeFloatRegister() = 0; //care not to take tmpAllocated

            std::vector< Instruction *> alloc_regs_;
            std::vector< Instruction *> alloc_fregs_;

            std::set<Register> tmpIntRegs_;
            std::set<FRegister> tmpFloatRegs_;

    };
}
