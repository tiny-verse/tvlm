#pragma once

#include "tvlm/tvlm/t86_backend.h"
#include "t86/instruction.h"
#include "t86/t86_target"
#include "t86/program.h"
#include "t86/program/programbuilder.h"




namespace tiny{
namespace tvlm{

class register_allocator {

    using Register = tiny::t86::Register;
    using FRegister = tiny::t86::FloatRegister;
    using TInstruction = tiny::t86::Instruction;
    using ILInstruction = ::tvlm::Instruction;

    register_allocator():
    regnum_(tiny::t86::Cpu::Config::instance().registerCnt()),
    fregnum_(tiny::t86::Cpu::Config::instance().floatRegisterCnt()),
//    regs_(regnum_, nullptr),
//    fregs_(fregnum_, nullptr),
    currentRegs_(regnum_, std::set<ILInstruction*>()),
    currentFRegs_(fregnum_, std::set<ILInstruction*>())
    {
        regs_.resize(regnum_);
        fregs_.resize(fregnum_);
        for(int i = 0; i < regnum_; i++){
            Register * tmp =
                    new tiny::t86::Register(i);
            regs_[i].reset(tmp);
            regIndex_.insert(std::make_pair(tmp, i));
        }
        for(int i = 0; i < fregnum_; i++){
            FRegister * tmp =
                    new tiny::t86::FloatRegister(i);
            fregs_[i].reset(tmp);
            fregIndex_.insert(std::make_pair(tmp, i));
        }
    }
    int regnum_;
    int fregnum_;
    std::vector<std::unique_ptr<Register>> regs_;
    std::vector<std::unique_ptr<FRegister>> fregs_;
    std::map<ILInstruction *, Register * > insToRegMapping_;
    std::map< Register *, std::set<ILInstruction *> > regToInsMapping_;
    std::map<ILInstruction *, FRegister * > insToFRegMapping_;
    std::map<FRegister *, ILInstruction *  > fregToInsMapping_;
    std::map<Register *, int> regIndex_;
    std::map<FRegister *, int> fregIndex_;

    std::vector<std::set<ILInstruction*>> currentRegs_;
    std::vector<std::set<ILInstruction*>> currentFRegs_;

    int findFreeReg(){
        for(int i = 0; i < regnum_;i++){
            if(currentRegs_[i].empty()){
                return i;
            }
        }
        return -1;
    }

    Register * getFreeReg(ILInstruction * ins){
        int tmp = findFreeReg();
        if(tmp == -1){
            tmp = findRegToSpill(ins);
            spillReg(tmp);
        }

        return tmp == -1 ? nullptr : regs_[tmp].get();
    }

    int findRegToSpill(ILInstruction * ins){

        return -1;
    }

    Register * Reg(int i)const {
        return regs_[i].get();
    }

    bool accommodateFreeReg(ILInstruction * ins){
        int i = findFreeReg();
        insToRegMapping_.insert(std::make_pair(ins, Reg(i) ));
        regToInsMapping_[Reg(i)].insert( ins );
        currentRegs_[0].insert(ins);
        //TODO
    }


    bool spillReg(int i){

        Register * regToSpill = regs_[i].get();

        for( ILInstruction * insToSpill : regToInsMapping_.find(regToSpill)->second){
            insToRegMapping_.erase(insToSpill);

        }
        //TODO insert into CODE?

        currentRegs_[i].clear();
        regToInsMapping_.erase(regToSpill);

        return true;
    }

    Register * getReg( ILInstruction * ins){

        Register * res = getFreeReg(ins);
        if(res) return res;



    }


//    int findFreeFReg(){
//        for(int i = 0; i < fregnum_;i++){
//            if(currentFRegs_[i].empty()){
//                return i;
//            }
//        }
//        return -1;
//    }
//    FRegister * getFreeFReg(ILIstruction * ins){
//
//        int tmp = findFreeFReg();
//        return tmp == -1 ? nullptr : fregs_[tmp].get();
//    }
//
//    FRegister * FReg(int i)const {
//        return fregs_[i].get();
//    }
//
//    bool accommodateFreeFReg(ILInstruction * ins){
//        int i = findFreeFReg();
//        insToFRegMapping_.insert(std::make_pair(ins, FReg(i) ));
//        fregToInsMapping_[FReg(i)].insert( ins );
//        currentFRegs_[0].insert(ins);
//    }
//
//
//    bool spillFReg(int i){
//
//        FRegister * regToSpill = fregs_[i].get();
//
//        for( ILInstruction * insToSpill : fregToInsMapping_.find(regToSpill)->second){
//            insToRegMapping_.erase(insToSpill);
//
//        }
//        //TODO insert into CODE?
//
//        currentRegs_[i].clear();
//        fregToInsMapping_.erase(regToSpill);
//
//        return true;
//    }
//
//    FRegister * getFReg( ILInstruction * ins){
//
//        FRegister * res = getFreeFReg(ins);
//        if(res) return res;
//
//
//    }
};



} //namespace tvlm
} //namespace tiny

