#include "t86_backend.h"
#include "t86/instruction.h"

namespace tvlm{

    void ILtoISNaive::visit(Instruction::Terminator0 * ins) {

    }

    void ILtoISNaive::visit(Instruction::Terminator1 * ins) {

    }

    void ILtoISNaive::visit(Instruction::TerminatorN * ins) {

    }

    void ILtoISNaive::visit(Instruction::Returnator * ins) {
        Label retVal = find(ins->returnValue());

        //TODO calling conventions
        add(ins, tiny::t86::RET());
    }

    void ILtoISNaive::visit(Instruction::DirectCallInstruction * ins) {

    }

    void ILtoISNaive::visit(Instruction::IndirectCallInstruction * ins) {

    }

    void ILtoISNaive::visit(Instruction::SrcInstruction * ins) {

    }

    void ILtoISNaive::visit(Instruction::BinaryOperator * ins) {

    }

    void ILtoISNaive::visit(Instruction::UnaryOperator * ins) {

    }

    void ILtoISNaive::visit(Instruction::ImmIndex * ins) {

    }

    void ILtoISNaive::visit(Instruction::ImmSize * ins) {

    }

    void ILtoISNaive::visit(Instruction::ImmValue * ins) {

    }

    void ILtoISNaive::visit(Instruction::VoidInstruction * ins) {

    }

    void ILtoISNaive::visit(Instruction::LoadAddress * ins) {

    }

    void ILtoISNaive::visit(Instruction::StoreAddress * ins) {

    }

    void ILtoISNaive::visit(Instruction::PhiInstruction * ins) {

    }
    void ILtoISNaive::visit(Instruction::ElemInstruction * ins) {

    }


    void ILtoISNaive::visit(BasicBlock * bb) {
        Label ret = Label::empty();
        for (auto & i : bb->insns_) {
            Label tmp = visitChild(i);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }

    void ILtoISNaive::visit(Function * fce) {
        Label ret = Label::empty();

        for (auto & bb :fce->bbs_) {
            Label tmp = visitChild(bb);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }

    void ILtoISNaive::visit(Program * p) {

        Label globals = visitChild(p->globals_);

        Label callMain = add(nullptr, tiny::t86::CALL{Label::empty()});
        for ( auto & f : p->functions_) {
            Label fncLabel = visitChild(f.second);
            functionTable_.emplace(f.first, fncLabel);
        }

        Label main = functionTable_.find(Symbol("main"))->second;
        pb_.patch(callMain, main);
        add(nullptr, tiny::t86::HALT{});

    }



    void ILTiler::visit(Instruction::Terminator0 * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        lastIns_ = new SpecializedDAG<Instruction::Terminator0>(ins, children );

    }

    void ILTiler::visit(Instruction::Terminator1 * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        lastIns_ = new SpecializedDAG<Instruction::Terminator1>(ins, children );

    }

    void ILTiler::visit(Instruction::TerminatorN * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        //TODO where all BBs?
        children.push_back(visitChild(ins->condition()));
        lastIns_ = new SpecializedDAG<Instruction::TerminatorN>(ins, children );

    }

    void ILTiler::visit(Instruction::Returnator * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->returnValue()));
        lastIns_ = new SpecializedDAG<Instruction::Returnator>(ins, children );

    }

    void ILTiler::visit(Instruction::DirectCallInstruction * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        for (auto & arg : ins->args()) {
            children.push_back(visitChild(arg));
        }
        lastIns_ = new SpecializedDAG<Instruction::DirectCallInstruction>(ins, children );
    }

    void ILTiler::visit(Instruction::IndirectCallInstruction * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->f()));
        for (auto & arg : ins->args()) {
            children.push_back(visitChild(arg));
        }
        lastIns_ = new SpecializedDAG<Instruction::IndirectCallInstruction>(ins, children );
    }

    void ILTiler::visit(Instruction::SrcInstruction * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->src()));
        lastIns_ = new SpecializedDAG<Instruction::SrcInstruction>(ins, children );

    }

    void ILTiler::visit(Instruction::BinaryOperator * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->lhs()));
        children.push_back(visitChild(ins->rhs()));
        lastIns_ = new SpecializedDAG<Instruction::BinaryOperator>(ins, children );



    }

    void ILTiler::visit(Instruction::UnaryOperator * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->operand()));
        lastIns_ = new SpecializedDAG<Instruction::UnaryOperator>(ins, children );
        return;
    }

    void ILTiler::visit(Instruction::ImmIndex * ins) {
        lastIns_ = new SpecializedDAG<Instruction::ImmIndex>(ins);
    }

    void ILTiler::visit(Instruction::ImmSize * ins) {
        lastIns_ = new SpecializedDAG<Instruction::ImmSize>(ins);
    }

    void ILTiler::visit(Instruction::ImmValue * ins) {
        lastIns_ = new SpecializedDAG<Instruction::ImmValue>(ins);
    }

    void ILTiler::visit(Instruction::VoidInstruction * ins) {
        lastIns_ = new SpecializedDAG<Instruction::VoidInstruction>(ins);
    }

    void ILTiler::visit(Instruction::LoadAddress * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->address()));
        lastIns_ = new SpecializedDAG<Instruction::LoadAddress>(ins, children );

    }

    void ILTiler::visit(Instruction::StoreAddress * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->address()));
        children.push_back(visitChild(ins->value()));
        lastIns_ = new SpecializedDAG<Instruction::StoreAddress>(ins, children );
    }

    void ILTiler::visit(Instruction::PhiInstruction * ins) {

    }

    void ILTiler::visit(Instruction::ElemInstruction *ins) {

    }

    void ILTiler::visit(BasicBlock * bb) {
        DAG * ret = nullptr;
        for (auto & i : bb->insns_) {
            DAG * tmp = visitChild(i);
            if(ret == nullptr){
                ret = tmp;
            }
        }
//        lastIns_ = visitChild(*(bb->insns_.end() -1));
        lastIns_ = ret;
    }

    void ILTiler::visit(Function * fce) {
        DAG*  ret = nullptr;

        for (auto & bb :fce->bbs_) {
            DAG* tmp = visitChild(bb);
            if(ret == nullptr){
                ret = tmp;
            }
        }
        lastIns_ = nullptr;
    }

    void ILTiler::visit(Program * p) {

        DAG* globs = visitChild(p->globals_);
        globals_ = globs;
        for ( auto & f : p->functions_) {
            visitChild(f.second);
        }

    }

//    void DAGBuilder::visit(Instruction::BinaryOperator * ins) {
//        DAG * tmp = add(ins);
//        visitChild(ins->lhs());
//        lastIns_->next_ = tmp;
//        visitChild(ins->rhs());
//        lastIns_->next_ = tmp;
//        lastIns_ = tmp;
//
//    }
    bool DummyRule::operator==(const DAG *other) const {
//        return *other == this;
        return true;
    }

    DummyRule *DummyRule::get() {
        if(dummy==nullptr){
            dummy = new DummyRule();
        }
        return dummy;
    }

} //namespace tvlm

#include "rewriting_rules.h"

std::unordered_map<tvlm::Instruction *, std::unordered_set<const tvlm::Rule*>> tvlm::ILTiler::allRules_ = AllRulesInit();

tvlm::DummyRule* tvlm::DummyRule::dummy = nullptr;

int tvlm::Rule::counter = 0;