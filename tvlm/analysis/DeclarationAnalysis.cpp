#include "DeclarationAnalysis.h"
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::Terminator0 *ins) {
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::Terminator1 *ins) {
//    visitChild(ins->getTarget(1));
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::Terminator2 *ins) {
//    visitChild(ins->getTarget(0));
//    visitChild(ins->getTarget(1));
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::Returnator *ins) {
//    visitChild(ins->returnValue());
////    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::DirectCallInstruction *ins) {
//    for ( auto & arg : ins->args()) {
//        visitChild(arg);
//    }
//    result_.insert(ins);
//
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::IndirectCallInstruction *ins) {
//    visitChild(ins->f());
//    for ( auto & arg : ins->args()) {
//        visitChild(arg);
//    }
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::SrcInstruction *ins) {
//    visitChild(ins->src());
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::BinaryOperator *ins) {
//    visitChild(ins->lhs());
//    visitChild(ins->rhs());
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::UnaryOperator *ins) {
//    visitChild(ins->operand());
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::ImmIndex *ins) {
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::ImmSize *ins) {
//    if(ins->amount()) visitChild(ins->amount());
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::ImmValue *ins) {
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::VoidInstruction *ins) {
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::LoadAddress *ins) {
//    visitChild(ins->address());
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::StoreAddress *ins) {
//    visitChild(ins->address());
//    visitChild(ins->value());
////    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::PhiInstruction *ins) {
//    for(auto & c :ins->contents()){
//       visitChild(c.second);
//    }
//    result_.insert(ins);
//}
//
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::ElemInstruction *ins) {
////    for (auto & e : ins->contents() ) {
////        if(e.first) visitChild(e.first);
////    }
////    result_.insert(ins); TODO
//}

void tvlm::DeclarationAnalysis::visit(tvlm::BasicBlock *bb) {

}

void tvlm::DeclarationAnalysis::visit(tvlm::Function *fce) {

}

void tvlm::DeclarationAnalysis::visit(tvlm::Program *p) {

}

void tvlm::DeclarationAnalysis::visit(tvlm::Jump *ins) {

}

void tvlm::DeclarationAnalysis::visit(tvlm::CondJump *ins) {

    result_.insert(ins->condition());

}

void tvlm::DeclarationAnalysis::visit(tvlm::Return *ins) {

    result_.insert(ins->returnValue());
}

void tvlm::DeclarationAnalysis::visit(tvlm::CallStatic *ins) {

//    result_.insert((IL *) program_->getGlobalVariableAddress(ins->f()->name()));
        // is injected or can be found no need to stay in register//
    for (auto & arg : ins->args()) {
        result_.insert(arg.first);
    }
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::Call *ins) {
    result_.insert(ins->f());
    for (auto & arg : ins->args()) {
        result_.insert(arg.first);
    }
    result_.insert(ins);

}

void tvlm::DeclarationAnalysis::visit(tvlm::Copy *ins) {

    result_.insert(ins->src());
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::Extend *ins) {

    result_.insert(ins->src());
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::Truncate *ins) {

}

void tvlm::DeclarationAnalysis::visit(tvlm::BinOp *ins) {

    result_.insert(ins->lhs());
    result_.insert(ins->rhs());
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::UnOp *ins) {

    result_.insert(ins->operand());
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::LoadImm *ins) {

    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::AllocL *ins) {
    if(ins->amount()){
        result_.insert(ins->amount());
        result_.insert(ins);
    }
}

void tvlm::DeclarationAnalysis::visit(tvlm::AllocG *ins) {
    if(ins->amount()){
        result_.insert(ins->amount());
        result_.insert(ins);
    }
//    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::ArgAddr *ins) {

    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::PutChar *ins) {

    result_.insert(ins->src());
}

void tvlm::DeclarationAnalysis::visit(tvlm::GetChar *ins) {

    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::Load *ins) {

    if(!dynamic_cast<AllocL*>(ins->address()) && !dynamic_cast<AllocG*>(ins->address()) ){
        // is Alloc .. ignore -> can be counted at use, does not need to be alive
        result_.insert(ins->address());
    }
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::Store *ins) {

    if(!dynamic_cast<AllocL*>(ins->address()) && !dynamic_cast<AllocG*>(ins->address()) ){
        result_.insert(ins->address());
    }
    result_.insert(ins->value());
}

void tvlm::DeclarationAnalysis::visit(tvlm::Phi *ins) {

    for(auto & incom : ins->contents()){
        result_.insert(incom.second);
    }
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::ElemAddrOffset *ins) {
    if (!dynamic_cast<AllocL *>(ins->base()) && !dynamic_cast<AllocG *>(ins->base())){
        result_.insert(ins->base());
    }
    result_.insert(ins->offset());
    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::ElemAddrIndex *ins) {
    if (!dynamic_cast<AllocL *>(ins->base()) && !dynamic_cast<AllocG *>(ins->base())){
        result_.insert(ins->base());
    }
    result_.insert(ins->index());
    result_.insert(ins->offset());

    result_.insert(ins);
}

void tvlm::DeclarationAnalysis::visit(tvlm::Halt *ins) {

}

void tvlm::DeclarationAnalysis::visit(tvlm::StructAssign *ins) {


    result_.insert(ins->dstAddr());
    result_.insert(ins->srcVal());
    result_.insert(ins);
}
