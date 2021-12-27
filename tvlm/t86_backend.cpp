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
        lastIns_ = new DAG(ins, ins->opcode_, children );

    }

    void ILTiler::visit(Instruction::Terminator1 * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        lastIns_ = new DAG(ins, ins->opcode_, children );

    }

    void ILTiler::visit(Instruction::TerminatorN * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        //TODO where all BBs?
        children.push_back(visitChild(ins->condition()));

        tvlm::Instruction::Opcode opcode;
        if(dynamic_cast<CondJump *>(ins)){
            opcode = tvlm::Instruction::Opcode::CondJump;
        }else{
            throw "unknown Opcode";
        }

        lastIns_ = new DAG(ins,opcode, children );

    }

    void ILTiler::visit(Instruction::Returnator * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->returnValue()));

                tvlm::Instruction::Opcode opcode = tvlm::Instruction::Opcode::Return;
        lastIns_ = new DAG(ins,opcode, children );

    }

    void ILTiler::visit(Instruction::DirectCallInstruction * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        for (auto & arg : ins->args()) {
            children.push_back(visitChild(arg));
        }

                tvlm::Instruction::Opcode opcode = tvlm::Instruction::Opcode::CallStatic;
        lastIns_ = new DAG(ins,opcode, children );
    }

    void ILTiler::visit(Instruction::IndirectCallInstruction * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->f()));
        for (auto & arg : ins->args()) {
            children.push_back(visitChild(arg));
        }

                tvlm::Instruction::Opcode opcode = tvlm::Instruction::Opcode::Call;
        lastIns_ = new DAG(ins,opcode, children );
    }

    void ILTiler::visit(Instruction::SrcInstruction * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->src()));

        tvlm::Instruction::Opcode opcode;
        if(dynamic_cast<PutChar *>(ins)){
            opcode = tvlm::Instruction::Opcode::PutChar;
        }else if(dynamic_cast<Truncate *>(ins)){
            opcode = tvlm::Instruction::Opcode::Truncate;
        }else if(dynamic_cast<Extend *>(ins)){
            opcode = tvlm::Instruction::Opcode::Extend;
        }else if(dynamic_cast<Copy *>(ins)){
            opcode = tvlm::Instruction::Opcode::Copy;
        }else{
            throw "unknown Opcode";
        }

        lastIns_ = new DAG(ins,opcode, children );

    }

    void ILTiler::visit(Instruction::BinaryOperator * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->lhs()));
        children.push_back(visitChild(ins->rhs()));

                tvlm::Instruction::Opcode opcode = tvlm::Instruction::Opcode::BinOp;
        lastIns_ = new DAG(ins,opcode, children );



    }

    void ILTiler::visit(Instruction::UnaryOperator * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->operand()));

                tvlm::Instruction::Opcode opcode = tvlm::Instruction::Opcode::UnOp;
        lastIns_ = new DAG(ins,opcode, children );
        return;
    }

    void ILTiler::visit(Instruction::ImmIndex * ins) {

        tvlm::Instruction::Opcode opcode;
        if(dynamic_cast<ArgAddr *>(ins)){
            opcode = tvlm::Instruction::Opcode::ArgAddr;
        }else{
            throw "unknown Opcode";
        }

        lastIns_ = new DAG(ins, opcode);
    }

    void ILTiler::visit(Instruction::ImmSize * ins) {

        tvlm::Instruction::Opcode opcode;
        if(dynamic_cast<AllocL *>(ins)){
            opcode = tvlm::Instruction::Opcode::AllocL;
        }else if(dynamic_cast<AllocG *>(ins)){
            opcode = tvlm::Instruction::Opcode::AllocG;
        }else{
            throw "unknown Opcode";
        }
        lastIns_ = new DAG(ins, opcode);
    }

    void ILTiler::visit(Instruction::ImmValue * ins) {

        tvlm::Instruction::Opcode opcode;
        if(dynamic_cast<LoadImm *>(ins)){
            opcode = tvlm::Instruction::Opcode::LoadImm;
        }else{
            throw "unknown Opcode";
        }

        lastIns_ = new DAG(ins, opcode);
    }

    void ILTiler::visit(Instruction::VoidInstruction * ins) {

        tvlm::Instruction::Opcode opcode;
        if(dynamic_cast<GetChar *>(ins)){
            opcode = tvlm::Instruction::Opcode::GetChar;
        }else{
            throw "unknown Opcode";
        }

        lastIns_ = new DAG(ins, opcode);
    }

    void ILTiler::visit(Instruction::LoadAddress * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->address()));

                tvlm::Instruction::Opcode opcode = tvlm::Instruction::Opcode::Load;
        lastIns_ = new DAG(ins,opcode, children );

    }

    void ILTiler::visit(Instruction::StoreAddress * ins) {
        std::vector<DAG*> children = std::vector<DAG*>();
        children.push_back(visitChild(ins->address()));
        children.push_back(visitChild(ins->value()));

                tvlm::Instruction::Opcode opcode = tvlm::Instruction::Opcode::Store;
        lastIns_ = new DAG(ins,opcode, children );
    }

    void ILTiler::visit(Instruction::PhiInstruction * ins) {

    }

    void ILTiler::visit(Instruction::ElemInstruction *ins) {

    }

    void ILTiler::visit(BasicBlock * bb) {
//        DAG * ret = nullptr;
        for (auto & i : bb->insns_) {
            DAG * tmp = visitChild(i);
//            if(ret == nullptr){
//                ret = tmp;
//            }
        }
//        lastIns_ = visitChild(*(bb->insns_.end() -1));
//        lastIns_ = ret;
    }

    void ILTiler::visit(Function * fce) {
        DAG*  ret = nullptr;

        for (auto & bb :fce->bbs_) {
            DAG* tmp = visitChild(bb);
            if(ret == nullptr){
                ret = tmp;
            }
        }
        lastIns_ = ret;

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

    void DAG::tile()  {
            //        auto & allRules = ILTiler::allRules_;
            //        //labels empty
            //        std::set<const Rule *> labels;
            //        std::vector<std::set<const Rule*>> tiledChildren;
            //        tiledChildren.resize(children_.size());
            //        for (int i = 0; i < children_.size(); i++) {
            //            tiledChildren[i] = children_[i]->tile();
            //        }
            //
            //
            ////            auto it = allRules.find(src_); // BAD
            //        auto it = allRules.begin(); //mathces type
            //        for(; it != allRules.end();it++ ){
            //            if( it->first == src_->opcode_){
            //                break;
            //            }
            //        }
            //        if(it == allRules.end()) {
            //            std::cerr << "no rules matches this instruction! ch:"<< children_.size() << "src: " << src_->name() << std::endl;
            //            return labels;
            //        }
            //        for (auto & r : it->second) { //all that  matches operation
            //            bool ret = r->matchesOpcode(src_->opcode_);
            //            for(int i = 0; i < tiledChildren.size();i++){
            //                auto & chLabels = tiledChildren[i];
            //                const Rule *  tmp = (*r)[i];
            //                auto chIt = chLabels.find(tmp); // find rule in
            //                if(chIt == chLabels.end() && ! dynamic_cast<const DummyRule *>(tmp)){
            //                    ret = false; break; //does not satisfy childs label
            //                }
            //            }
            //            if(ret){ // all satisfied
            //                labels.emplace(r.get());
            //            }
            //        }
            //
            //
            //        return labels;


        auto & allRules = ILTiler::allRules_;
//
//        std::vector<std::set<const Rule*>> tiledChildren;
//        tiledChildren.resize(children_.size());
//        for (int i = 0; i < children_.size(); i++) {
//            tiledChildren[i] = children_[i]->labels_;
//        }

        auto it = allRules.begin(); //mathces type
        for(; it != allRules.end();it++ ){
            if( it->first == src_->opcode_){
                break;
            }
        }
        if(it == allRules.end()) {
            std::cerr << "no rules matches this instruction! ch:"<< children_.size() << "src: " << src_->name() << std::endl;

            return ;
        }
        for (auto & r : it->second) { //all that  matches operation
            bool ret = r->matchesOpcode(src_); // matches instruction
            if(ret && *r == this){ // all satisfied (+ rule can cover dag)
                labels_.emplace(r->makeCopy());
            }
        }

        return ;
    }

    bool SpecializedRule::operator==(const DAG *other) const {
        return (customMatcher_ ) ?customMatcher_(this, other) :  matchRuleToDAG(other);

    }

    bool SpecializedRule::matchRuleToDAG(const DAG *dag) const {
        //has same Opcode
        bool ret = dag->matchesOpcode(this->opcode_);
        if(!ret) return false;
        //has same arity
        if(children_.size() != dag->children_.size()) return false;
        //can cover dag;
        const SpecializedRule * rule = nullptr;
        for( int i = 0; i < children_.size();i++){
            auto & chRule = children_[i];
            if(dynamic_cast<const DummyRule *>(chRule)){continue;}
            if((rule  = dynamic_cast<const SpecializedRule *>(chRule))){
                if( !rule->matchRuleToDAG(dag->children_[i])){
                    return false;
                }else{
                    continue;
                }

            }

            return false;
        }



        return true;
    }
} //namespace tvlm

#include "rewriting_rules.h"


std::unordered_map<Opcode, std::set<std::unique_ptr<tvlm::Rule>>> tvlm::ILTiler::allRules_ = AllRulesInit();

tvlm::DummyRule* tvlm::DummyRule::dummy = nullptr;

int tvlm::Rule::counter = 0;