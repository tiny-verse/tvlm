#pragma once

#include "tvlm/il/il.h"
#include "t86/program.h"
#include "t86/instruction.h"
#include "t86/program/label.h"

#include "tvlm/codeGeneration/ProgramBuilder.h"

namespace tvlm{

class Epilogue : public TargetProgramFriend{
public:
    using TProgram = tiny::t86::Program;
    using SProgram = tvlm::TargetProgram;


    virtual ~Epilogue() = default;
    Epilogue(){

    }

     virtual TProgram translate( SProgram && program) = 0;


protected:

};


class NaiveEpilogue : public Epilogue, public ILVisitor{
public:
    TProgram  translate(SProgram && program) override;
//    TProgram translate() {
//        return std::move(translate(program_));
//    }

    virtual ~NaiveEpilogue() = default;
    NaiveEpilogue():program_(), lastIns_(Label::empty()){}
protected:
    void visitInstrHelper(Instruction *ins);
    Label add(const Instruction * ins);
    Label compiledGlobalTable(BasicBlock *globals);
    Label resolveInstruction(const std::pair<const Instruction *, Label> & pos)const {
        auto it = compiledInsns_.find(pos);
        if(it == compiledInsns_.end()){
            throw "[Epilogue] - resolveInstruction - instruction could not be found";
            return Label::empty();
        }else{
            return (it->second );
        }
//        auto it = compiledInsns_.find(pos.first);
//        if(it == compiledInsns_.end()){
//            throw "instruction could not be found";
//            return Label::empty();
//        }else{
//            return (it->second + pos.second);
//        }
    }
    int64_t getFuncAlloc(Function * fnc){
        auto alloc = getFuncLocalAlloc(&program_);
        auto it = alloc.find(fnc);
        if(it == alloc.end()){
            throw "function not found in localAlloc";
            return -1;
        }else{
            return (int64_t)it->second;
        }
    }

private:
    ProgramBuilder pb_;
    SProgram  program_;
    Label lastIns_;
    std::unordered_map<tiny::Symbol, Label> functionTable_;
    std::unordered_map<const BasicBlock *, Label> compiledBB_;
    std::map<std::pair<const Instruction *, Label>, Label> compiledInsns_;
//    std::map<const Instruction *, Label> compiledInsns_;
public:
    void visit(Instruction *ins) override;
    void visit(Jump *ins) override;
    void visit(CondJump *ins) override;
    void visit(Return *ins) override;
    void visit(CallStatic *ins) override;
    void visit(Call *ins) override;
    void visit(Copy *ins) override;
    void visit(Extend *ins) override;
    void visit(Truncate *ins) override;
    void visit(BinOp *ins) override;
    void visit(UnOp *ins) override;
    void visit(LoadImm *ins) override;
    void visit(AllocL *ins) override;
    void visit(AllocG *ins) override;
    void visit(ArgAddr *ins) override;
    void visit(PutChar *ins) override;
    void visit(GetChar *ins) override;
    void visit(Load *ins) override;
    void visit(Store *ins) override;
    void visit(Phi *ins) override;
    void visit(ElemAddrOffset *ins) override;
    void visit(ElemAddrIndex *ins) override;
    void visit(Halt *ins) override;
    void visit(StructAssign *ins) override;
    void visit(BasicBlock *bb) override;
    void visit(Function *fce) override;
    void visit(Program *p) override;

    };

}
