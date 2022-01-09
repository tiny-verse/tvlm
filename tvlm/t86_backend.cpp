#include "t86_backend.h"
#include "t86/instruction.h"
#include "analysis/liveness_analysis.h"

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
        for (auto & i : getBBsInstructions(bb)) {
            Label tmp = visitChild(i);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }

    void ILtoISNaive::visit(Function * fce) {
        Label ret = Label::empty();

        for (auto & bb : getFunctionBBs(fce)) {
            Label tmp = visitChild(bb);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }

    void ILtoISNaive::visit(Program * p) {

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
        std::vector<DAG*> children = std::vector<DAG*>();
        if(ins->multiply()){
            children.emplace_back(visitChild(ins->multiply()));
        }
        lastIns_ = new DAG(ins, opcode, children);
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
        for (auto & i : getBBsInstructions(bb)) {
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

        for (auto & bb : getFunctionBBs(fce)) {
            DAG* tmp = visitChild(bb);
            if(ret == nullptr){
                ret = tmp;
            }
        }
        lastIns_ = ret;

    }

    void ILTiler::visit(Program * p) {

        DAG* globs = visitChild(getProgramsGlobals(p));
        globals_.reset(globs);
        for ( auto & f : getProgramsFunctions(p)) {
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
    bool DummyRule::operator==(const DAG *other) {
//        return *other == this;
        return true;
    }

    int DummyRule::countCost() {
        return 1;
    }

//    DummyRule *DummyRule::get() {
//        if(dummy==nullptr){
//            dummy = new DummyRule();
//        }
//        return dummy;
//    }
//    DummyRule *DummyRule::getnew() {
//        return new DummyRule();
//    }

//    void DAG::tile()  {
////                    auto & allRules = ILTiler::allRules_;
////                    //labels empty
////                    std::set<const Rule *> labels;
////                    std::vector<std::set<const Rule*>> tiledChildren;
////                    tiledChildren.resize(children_.size());
////                    for (int i = 0; i < children_.size(); i++) {
////                        tiledChildren[i] = children_[i]->tile();
////                    }
////
////
////            //            auto it = allRules.find(src_); // BAD
////                    auto it = allRules.begin(); //mathces type
////                    for(; it != allRules.end();it++ ){
////                        if( it->first == src_->opcode_){
////                            break;
////                        }
////                    }
////                    if(it == allRules.end()) {
////                        std::cerr << "no rules matches this instruction! ch:"<< children_.size() << "src: " << src_->name() << std::endl;
////                        return labels;
////                    }
////                    for (auto & r : it->second) { //all that  matches operation
////                        bool ret = r->matchesOpcode(src_->opcode_);
////                        for(int i = 0; i < tiledChildren.size();i++){
////                            auto & chLabels = tiledChildren[i];
////                            const Rule *  tmp = (*r)[i];
////                            auto chIt = chLabels.find(tmp); // find rule in
////                            if(chIt == chLabels.end() && ! dynamic_cast<const DummyRule *>(tmp)){
////                                ret = false; break; //does not satisfy childs label
////                            }
////                        }
////                        if(ret){ // all satisfied
////                            labels.emplace(r.get());
////                        }
////                    }
////
////
////                    return labels;
//
//
//        auto & allRules = ILTiler::allRules_;
////
////        std::vector<std::set<const Rule*>> tiledChildren;
////        tiledChildren.resize(children_.size());
////        for (int i = 0; i < children_.size(); i++) {
////            tiledChildren[i] = children_[i]->labels_;
////        }
//
//        auto it = allRules.begin(); //mathces type
//        for(; it != allRules.end();it++ ){
//            if( it->first == src_->opcode_){
//                break;
//            }
//        }
//
//
//        if(it == allRules.end()) {
//            std::cerr << "no rules matches this instruction! ch:"<< children_.size() << "src: " << src_->name() << std::endl;
//
//            return ;
//        }
//        for (auto & r : it->second) { //all that  matches operation
//            bool ret = r->matchesOpcode(src_); // matches instruction
//            if(ret && *r == this){ // all satisfied (+ rule can cover dag)
//                labels_.emplace(r->makeCopy());
//            }
//        }
//
//        return ;
//    }


    void DAG::tile()  {

        auto & allRules = ILTiler::allRules_;

        auto it =  allRules.find(src_->opcode_); //matching Type
        if(it == allRules.end()) {
            std::cerr << "no rules matches this instruction! ch:"<< children_.size() << "src: " << src_->name() << std::endl;

            return ;
        }
        for (auto & r : it->second) { //all that  matches operation
            r       ;
            r->matchesOpcode(src_->opcode_); // matches instruction
            bool ret =
                    r->matchesOpcode(src_->opcode_); // matches instruction
            if(ret && *r == this){ // all satisfied (+ rule can cover dag)
                auto tmp = r->makeCopy();
                labels_.emplace(tmp);
            }
        }

        return ;
    }

//    bool SpecializedRule::operator==(const DAG *other) const {
//        return (customMatcher_ ) ?customMatcher_(this, other) :  matchRuleToDAG(other);
//
//    }
//
//    bool SpecializedRule::matchRuleToDAG(const DAG *dag) const {
//        //has same Opcode
//        bool ret = dag->matchesOpcode(this->opcode_);
//        if(!ret) return false;
//        //has same arity
//        if(children_.size() != dag->children_.size()) return false;
//        //can cover dag;
//        const SpecializedRule * rule = nullptr;
//        for( int i = 0; i < children_.size();i++){
//            auto & chRule = children_[i];
//            if(dynamic_cast<const DummyRule *>(chRule.get())){continue;}
//            if((rule  = dynamic_cast<const SpecializedRule *>(chRule.get()))){
//                if( !rule->matchRuleToDAG(dag->children_[i])){
//                    return false;
//                }else{
//                    continue;
//                }
//
//            }
//
//            return false;
//        }
//
//
//
//        return true;
//    }
} //namespace tvlm

#include "rewriting_rules.h"
#include "tvlm/analysis/next_use_analysis.h"


std::map<Opcode, std::set<tvlm::Rule*>> tvlm::ILTiler::allRules_ = [](){
    std::map<Opcode, std::set<tvlm::Rule*>> tmp;

    auto rules = AllRulesInit();
    std::vector<tvlm::Opcode> opcodes = {tvlm::Opcode::ADD,
                                         tvlm::Opcode::SUB,
                                         tvlm::Opcode::UNSUB,
                                         tvlm::Opcode::MOD,
                                         tvlm::Opcode::MUL,
                                         tvlm::Opcode::DIV,
                                         tvlm::Opcode::AND,
                                         tvlm::Opcode::OR,
                                         tvlm::Opcode::XOR,
                                         tvlm::Opcode::NOT,
                                         tvlm::Opcode::LSH,
                                         tvlm::Opcode::RSH,
                                         tvlm::Opcode::INC,
                                         tvlm::Opcode::DEC,
                                         tvlm::Opcode::NEQ,
                                         tvlm::Opcode::EQ,
                                         tvlm::Opcode::LTE,
                                         tvlm::Opcode::LT,
                                         tvlm::Opcode::GT,
                                         tvlm::Opcode::GTE,
                                         tvlm::Opcode::BinOp,
                                         tvlm::Opcode::UnOp,
                                         tvlm::Opcode::LoadImm,
                                         tvlm::Opcode::AllocG,
                                         tvlm::Opcode::AllocL,
                                         tvlm::Opcode::Halt,
                                         tvlm::Opcode::Return,
                                         tvlm::Opcode::GetChar,
                                         tvlm::Opcode::PutChar,
                                         tvlm::Opcode::CondJump,
                                         tvlm::Opcode::Jump,
                                         tvlm::Opcode::Phi,
                                         tvlm::Opcode::Call,
                                         tvlm::Opcode::CallStatic,
                                         tvlm::Opcode::Extend,
                                         tvlm::Opcode::Truncate,
                                         tvlm::Opcode::ElemAddr,
                                         tvlm::Opcode::Copy,
                                         tvlm::Opcode::Store,
                                         tvlm::Opcode::Load,
                                         tvlm::Opcode::ArgAddr
    };

    for (auto & rule : rules) {
        for (auto & op: opcodes) {
            if(rule->matchesOpcode(op)){
                tmp[op].insert(rule.get());
            }
        }
    }
    return tmp;

}();


tiny::t86::Program tvlm::ILTiler::translate(tvlm::Program & prog){
    /*STEPS:
            * Create DAG
            * Optimize DAG
            * Tile DAG
            * Analyse live ranges
            * Resolve reg Allocation
            * Complete ASSEMBLY
    */


    //TODO
//    ILTiler v;
//    v.visit( &prog);//create dag

    //analyse
    auto la = std::unique_ptr<LivenessAnalysis>(LivenessAnalysis::create(&prog));
    auto analysis = la->analyze();
    auto nla = std::unique_ptr<NextUseAnalysis>(NextUseAnalysis::create(&prog));
    auto nanalysis = nla->analyze();
    std::cerr << "huh" << std::endl;
//    v.functionTable_.begin()->second->tile();

//    auto & globLabels = v.functionTable_.begin()->second->labels_;

//    std::cerr << "huh" << globLabels.size() << std::endl;
    tiny::t86::ProgramBuilder pb;
    return std::move(pb.program());
}


//tvlm::DummyRule* tvlm::DummyRule::dummy = nullptr;
int tvlm::Rule::counter = 0;