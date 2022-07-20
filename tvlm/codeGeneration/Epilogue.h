#pragma once

#include "tvlm/il/il.h"
#include "t86/program.h"
#include "t86/instruction.h"
#include "t86/program/label.h"

#include "tvlm/codeGeneration/ProgramBuilder.h"

namespace tvlm{

class Epilogue {
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
    TProgram translate(SProgram &&program) override;

    ~NaiveEpilogue() override = default;
private:
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
