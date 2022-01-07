#include "instruction_analysis.h"

tvlm::VirtualRegisterPlaceholder * tvlm::InstructionAnalysis::InsVisitor::getVirtualReg(const tvlm::Declaration pIl) {
            auto it = virtualRegs_.find(pIl);

    if(it != virtualRegs_.end()){
        return it->second.get();
    }else{
        auto tmp = new VirtualRegister();
        virtualRegs_.emplace(pIl, tmp);
        return tmp;
    }
}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::Terminator0 *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::Terminator1 *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::TerminatorN *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::Returnator *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::DirectCallInstruction *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::IndirectCallInstruction *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::SrcInstruction *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::BinaryOperator *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::UnaryOperator *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ImmIndex *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ImmSize *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ImmValue *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::VoidInstruction *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::LoadAddress *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::StoreAddress *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::PhiInstruction *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ElemInstruction *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(::tvlm::BasicBlock *bb) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Function *fce) {
//    auto extEnv = extendEnv(env_, fce->);
//    visitChild(basifce->)
}


void tvlm::InstructionAnalysis::InsVisitor::visit(Program *p) {
    std::vector<IL * > tmp;
    for(const auto & f : p->functions()){
        tmp.emplace_back(f.second.get());
    }
    auto extEnv = extendEnv(env_, tmp);
    for (const auto & f : p->functions()) {
        visitChild(f.second, extEnv);
    }
}

tvlm::InstructionAnalysis::Env tvlm::InstructionAnalysis::InsVisitor::extendEnv(tvlm::InstructionAnalysis::Env &env,
                                                                                const std::vector<Declaration> &decls) {
    auto acc = env;
    for (const auto & d : decls) {
        VirtualRegisterPlaceholder * reg = getVirtualReg(d);
        auto tmp = acc.access(reg);
        if(!tmp){
            acc.insert(std::make_pair( reg, d));
        }
    }
    return acc;
}
