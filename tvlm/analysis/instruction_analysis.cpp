#include "instruction_analysis.h"

tvlm::VirtualRegisterPlaceholder * tvlm::InstructionAnalysis::InsVisitor::getVirtualReg(const tvlm::Declaration *pIl) {
            auto it = virtualRegs_.find(pIl); // TODO

            if(it != virtualRegs_.end()){
                return it->second.get();
            }else{
                auto tmp = new VirtualRegisterPlaceholder(RegisterType::INTEGER, counter_++);
                virtualRegs_.emplace(pIl, tmp);
                return tmp;
            }
}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::Terminator0 *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::Terminator1 *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::Terminator2 *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::Returnator *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::DirectCallInstruction *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::IndirectCallInstruction *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::SrcInstruction *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::BinaryOperator *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::UnaryOperator *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ImmIndex *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ImmSize *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ImmValue *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::VoidInstruction *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::LoadAddress *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::StoreAddress *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::PhiInstruction *ins) {
//
//}
//
//void tvlm::InstructionAnalysis::InsVisitor::visit(Instruction::ElemInstruction *ins) {
//
//}

void tvlm::InstructionAnalysis::InsVisitor::visit(::tvlm::BasicBlock *bb) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(Function *fce) {
//    auto extEnv = extendEnv(env_, fce->);
//    visitChild(basifce->)
}


void tvlm::InstructionAnalysis::InsVisitor::visit(Program *p) {
    std::vector<IL * > globs;
    for(auto * il : getBBsInstructions(getProgramsGlobals(p))){
        if(dynamic_cast<AllocG *>(il) || dynamic_cast<Store *>(il)){
           continue;
        }
        globs.emplace_back(il);
    }
//    auto extEnv1 = extendEnv(env_, globs);
    std::vector<IL * > tmp;
    for(const auto & f : p->functions()){
        tmp.emplace_back(f.second.get());
    }
//    auto extEnv2 = extendEnv(extEnv1, tmp);
    for (const auto & f : p->functions()) {
//        visitChild(f.second, extEnv2);
        ILVisitor::visitChild(f.second.get());
    }
}

tvlm::InstructionAnalysis::Env tvlm::InstructionAnalysis::InsVisitor::extendEnv(tvlm::InstructionAnalysis::Env &env,
                                                                                const std::vector<Declaration*> &decls) {
    auto acc = env;
    for (IL * d : decls) {
        VirtualRegisterPlaceholder * reg = getVirtualReg(d);
        Declaration * tmp = acc.access(reg);
        if(!tmp){
            acc.emplace( reg, d);
        }
    }
    return acc;
}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Jump *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::CondJump *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Return *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::CallStatic *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Call *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Copy *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Extend *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Truncate *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::BinOp *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::UnOp *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::LoadImm *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::AllocL *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::AllocG *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::ArgAddr *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::PutChar *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::GetChar *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Load *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Store *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Phi *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::ElemAddrOffset *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::ElemAddrIndex *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::Halt *ins) {

}

void tvlm::InstructionAnalysis::InsVisitor::visit(tvlm::StructAssign *ins) {

}

size_t tvlm::InstructionAnalysis::counter_ = 0;