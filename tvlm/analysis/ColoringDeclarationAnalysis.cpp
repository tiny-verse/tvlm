#include "ColoringDeclarationAnalysis.h"


void tvlm::ColoringDeclarationAnalysis::visit(tvlm::BasicBlock *bb) {

}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Function *fce) {

}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Program *p) {

}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Jump *ins) {

}

void tvlm::ColoringDeclarationAnalysis::visitHelper(const Instruction * ins){

    auto it = getAllocatedRegisters(program_).find(ins);
    if(it!= getAllocatedRegisters(program_).end()){

        for (auto & i : it->second){
            result_.insert(&i);
        }
    }
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::CondJump *ins) {

    visitHelper(ins->condition());
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Return *ins) {

    visitHelper(ins->returnValue());
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::CallStatic *ins) {

    visitHelper( getProgram(program_)->getGlobalVariableAddress(ins->f()->name()));
    for (auto & arg : ins->args()) {
        visitHelper(arg.first);
    }
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Call *ins) {
    visitHelper(ins->f());
    for (auto & arg : ins->args()) {
        visitHelper(arg.first);
    }
    visitHelper(ins);

}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Copy *ins) {

    visitHelper(ins->src());
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Extend *ins) {

    visitHelper(ins->src());
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Truncate *ins) {

}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::BinOp *ins) {

    visitHelper(ins->lhs());
    visitHelper(ins->rhs());
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::UnOp *ins) {

    visitHelper(ins->operand());
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::LoadImm *ins) {

    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::AllocL *ins) {

    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::AllocG *ins) {

    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::ArgAddr *ins) {

    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::PutChar *ins) {

    visitHelper(ins->src());
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::GetChar *ins) {

    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Load *ins) {

    visitHelper(ins->address());
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Store *ins) {

    visitHelper(ins->address());
    visitHelper(ins->value());
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Phi *ins) {

    for(auto & incom : ins->contents()){
        visitHelper(incom.second);
    }
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::ElemAddrOffset *ins) {
    visitHelper(ins->offset());
    visitHelper(ins->base());
    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::ElemAddrIndex *ins) {

    visitHelper(ins->base());
    visitHelper(ins->index());
    visitHelper(ins->offset());

    visitHelper(ins);
}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::Halt *ins) {

}

void tvlm::ColoringDeclarationAnalysis::visit(tvlm::StructAssign *ins) {


    visitHelper(ins->dstAddr());
    visitHelper(ins->srcVal());
    visitHelper(ins);
}
