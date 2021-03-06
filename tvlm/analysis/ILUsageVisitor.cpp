#include "ILUsageVisitor.h"
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
//void tvlm::ILUsageVisitor::visit(tvlm::Instruction::TerminatorN *ins) {
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

void tvlm::ILUsageVisitor::visit(tvlm::BasicBlock *bb) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Function *fce) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Program *p) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Jump *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::CondJump *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Return *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::CallStatic *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Call *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Copy *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Extend *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Truncate *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::BinOp *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::UnOp *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::LoadImm *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::AllocL *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::AllocG *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::ArgAddr *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::PutChar *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::GetChar *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Load *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Store *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Phi *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::ElemAddrOffset *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::ElemAddrIndex *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::Halt *ins) {

}

void tvlm::ILUsageVisitor::visit(tvlm::StructAssign *ins) {

}
