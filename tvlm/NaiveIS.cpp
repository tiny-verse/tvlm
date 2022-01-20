#include "NaiveIS.h"
#include "t86/program/helpers.h"
#include "t86/instruction.h"

namespace tvlm{

    void NaiveIS::visit(Instruction *ins) {}
    
    void NaiveIS::visit(Halt *ins) {}
    
    void NaiveIS::visit(Jump *ins) {
        tiny::t86::Label jmp = add(tiny::t86::JMP( tiny::t86::Label::empty()));
        future_patch_.emplace_back(jmp, ins->getTarget(1));
//        return jmp;
        lastIns_ = jmp;
    }
    
    void NaiveIS::visit(CondJump *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::CMP(fillIntRegister(ins->condition()), 0));
        clearInt(ins->condition());
        tiny::t86::Label condJump = add(tiny::t86::JZ(tiny::t86::Label::empty()));
        tiny::t86::Label jumpToTrue = add(tiny::t86::JMP(tiny::t86::Label::empty()));
//        t86::Label jumpToFalse = add(t86::JMP(t86::Label::empty()));
//        patch(condJump, jumpToFalse);

        future_patch_.emplace_back(jumpToTrue, ins->getTarget(1));
        future_patch_.emplace_back(/*jumpToFalse*/condJump, ins->getTarget(0));

//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(Return *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::MOV(tiny::t86::Reg(0 /*or Somwhere on stack*/  ), fillIntRegister(ins->returnValue())));
        clearInt(ins->returnValue());
        add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()));
        add(tiny::t86::POP(tiny::t86::Bp()));
        add(tiny::t86::RET());

        lastIns_= ret;
    }
    
    void NaiveIS::visit(CallStatic *ins) {
    
    }
    
    void NaiveIS::visit(Call *ins) {
    
    }

    void NaiveIS::visit(BinOp *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::SUB(fillIntRegister(ins->lhs()), fillIntRegister(ins->rhs())));

        add(tiny::t86::MOV(fillIntRegister(ins->rhs()), tiny::t86::Flags()));
        add(tiny::t86::AND(fillIntRegister(ins->rhs()),0x01));//SignFlag
        regAllocator->alloc_regs_[getIntRegister(ins->rhs()).index()] = ins;
        clearInt(ins->rhs());
        lastIns_ = ret;//        return ret;
    }
    
    void NaiveIS::visit(UnOp *ins) {
    
    }
    
    void NaiveIS::visit(LoadImm *ins) {
    
    }
    
    void NaiveIS::visit(ArgAddr *ins) {
    
    }
    
    void NaiveIS::visit(AllocL *ins) {
    
    }

    void NaiveIS::visit(AllocG *ins) {

    }


    void NaiveIS::visit(Copy *ins) {

    }

    void NaiveIS::visit(Extend *ins) {

    }

    void NaiveIS::visit(Truncate *ins) {

    }


    void NaiveIS::visit(PutChar *ins) {

    }

    void NaiveIS::visit(GetChar *ins) {

    }

    void NaiveIS::visit(Load *ins) {

    }

    void NaiveIS::visit(Store *ins) {

    }

    void NaiveIS::visit(Phi *ins) {

    }

    void NaiveIS::visit(ElemAddrOffset *ins) {

    }

    void NaiveIS::visit(ElemAddrIndex *ins) {

    }
    
    void NaiveIS::visit(BasicBlock * bb) {
        Label ret = Label::empty();
        for (auto & i : getBBsInstructions(bb)) {
            Label tmp = visitChild(i);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(Function * fce) {
        Label ret = Label::empty();
    
        for (auto & bb : getFunctionBBs(fce)) {
            Label tmp = visitChild(bb);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(Program * p) {
    
        Label globals = visitChild(getProgramsGlobals(p));
    
        Label callMain = add(nullptr, tiny::t86::CALL{Label::empty()});
        for ( auto & f : getProgramsFunctions(p)) {
            Label fncLabel = visitChild(f.second);
            functionTable_.emplace(f.first, fncLabel);
        }
    
        Label main = functionTable_.find(Symbol("main"))->second;
        pb_.patch(callMain, main);
        add(nullptr, tiny::t86::HALT{});
    
    }

    tiny::t86::Program NaiveIS::translate(Program &prog) {
        //TODO
        NaiveIS v;
        v.visit( &prog);
        tiny::t86::Program rawProg = v.pb_.program();
        std::vector<tiny::t86::Instruction*> instrs = rawProg.moveInstructions();
        int line = 0;
        for(const tiny::t86::Instruction * i : instrs){
            std::cerr << tiny::color::blue << line++ << ": " << tiny::color::green << i->toString() << std::endl;
        }


        return {instrs, rawProg.data()};
//            return v.pb_.program();
    }



}
