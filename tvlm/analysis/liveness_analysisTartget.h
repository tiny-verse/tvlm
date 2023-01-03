#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "cfg.h"
#include "DeclarationAnalysis.h"


namespace tvlm{

class CLiveRange {
    using IL = ::tvlm::IL;
protected:
//    CLiveRange(const std::set<IL *>& il,ResultType & type, IL * start, IL * end):
//            il_(il),
//            type_(type),
//            start_(start),
//            end_(end){
//        assert(il_.size() > 0);
//        for(auto * i : il_){
//            auto ins = dynamic_cast<Instruction*>(i);
//            assert(ins && ins->resultType() == type_)
//        }
//    }
public:
    CLiveRange(Instruction * il, IL * end):
    il_({il}),
    type_(il->resultType()),
    start_(end),
    end_(end){

    }

    void add(Instruction * in) {
        il_.emplace(in);
    }

    CLiveRange * merge( CLiveRange * other) { // always care for merge order
        if(type() != other->type()){
            throw "[livenessAnalysisTarget]merging LiveRange of different types";
        }
        if(il_ != other->il_){
            il_.merge( other->il_);
        }
        start_ = other->start_;

        //just to be safe
        other->start_ = start_;
        other->end_ = end_;
        other->il_ = il_;
        //

        return this;
    }

    void setStart(IL *start){
        start_ = start;
    }
    void setEnd(IL *end){
        end_ = end;
    }
    ResultType type()const{
        return type_;
    }

    const std::set<IL *>& il() const {
        return il_;
    }
    IL * start() const {
        return start_;
    }
    IL * end() const {
        return end_;
    }
private:
    std::set<IL *> il_;
    ResultType type_;
    IL * start_;
    IL * end_;
};


//class CliveRangeComparator{
//public:
//    bool operator()(const CLiveRange * l, const CLiveRange * r)const{
//        return l->il() < r->il();
//    }
//    bool operator()(const Instruction * l, const CLiveRange * r)const{
//        return l < r->start();
//    }
//    bool operator()(const CLiveRange * l, const Instruction * r)const{
//        return l->start() < r;
//    }
//};

    using Instruction = ::tvlm::Instruction;

    template<class T>
    using CLiveVars = MAP<const CfgNode<T> *, std::set<CLiveRange*>>;
    ;

    template<class Info = DummyClass>
    class ColoringLiveAnalysis : public BackwardAnalysis<CLiveVars<Info>, Info>{
    protected:
        std::shared_ptr<Program>  getProgram(TargetProgram * p)const {
            return TargetProgramFriend::getProgram(p);
        }
    private:
//        using Declaration = tvlm::Instruction;
        using NodeState = std::set<CLiveRange*>;
        //    using Declarations = MAP< ILInstruction *, Declaration*>;

        NodeState join(const CfgNode<Info> * node, CLiveVars<Info> & state);

        ColoringLiveAnalysis( const Declarations & declarations, TargetProgram * program);

        std::set<CLiveRange*> getSubtree(const CfgNode<Info> *pNode);

        NodeState transferFun(const CfgNode<Info> * node, const  NodeState & state );

        void combineLR( NodeState & newState, Instruction * ins, Instruction * otherIns);

        NodeState funOne(const CfgNode<Info> * node, CLiveVars<Info> & state);
    public:
//        static LivenessAnalysisTMP<Info> * create(Program * p);
        virtual ~ColoringLiveAnalysis(){
            delete cfg_;
        }
        explicit ColoringLiveAnalysis(TargetProgram * p);
        CLiveVars<Info> analyze() override;
        std::vector<CLiveRange*> getLiveRanges(){
            return std::vector<CLiveRange*>(allocatedLR_.begin(), allocatedLR_.end());
        }

    private:
        std::set<CLiveRange*> allocatedLR_;
        std::map<const IL*, CLiveRange *> varMaps_;
        std::map<CLiveRange *, CLiveRange *> deletedLR_; // delete key, replaced with value
        ProgramCfg<Info> * cfg_;
        NodeState allVars_;
        CPowersetLattice<CLiveRange*> nodeLattice_;
        MapLattice<const CfgNode<Info> *, NodeState> lattice_;
        TargetProgram * program_;
        DeclarationAnalysis declAnalysis_;

        bool canBinOpCombine(BinOp * binOp) {

            if( binOp->lhs()->resultType() == ResultType::Integer ||
                binOp->lhs()->resultType() == ResultType::StructAddress){
                return true;
            }else if(binOp->lhs()->resultType() == ResultType::Double){
                switch (binOp->opType()){
                    case BinOpType::LT:
                    case BinOpType::LTE:
                    case BinOpType::GT:
                    case BinOpType::GTE:
                    case BinOpType::EQ:
                    case BinOpType::NEQ:
                    case BinOpType::AND:
                    case BinOpType::OR:
                        return false;
                    default:
                        return true;
                }
            }
            return false;

        }
    };
//************************************************************************************************************

    template<class I>
    ColoringLiveAnalysis<I>::ColoringLiveAnalysis(TargetProgram *p):
            ColoringLiveAnalysis( InstructionAnalysis(getProgram(p).get()).analyze(), p)
    {
    }

    template<class I>
    std::set<CLiveRange*> ColoringLiveAnalysis<I>::getSubtree(const CfgNode<I> *node) {
//        DeclarationAnalysis v(getProgram(program_));
        std::set<CLiveRange*> res;
        declAnalysis_.begin(node->il());
        auto children = declAnalysis_.result();
        children.erase(node->il());
        for (const auto * ch : children) {
            auto lr = varMaps_.find(ch);
            if(lr != varMaps_.end()){
                   lr->second->setEnd(node->il());
                   res.emplace(lr->second);
            }
        }
        auto lr = varMaps_.find(node->il());
        if(lr != varMaps_.end()){
            res.emplace(lr->second);
        }
        return res;
    }

    template<class I>
    void ColoringLiveAnalysis<I>::combineLR( ColoringLiveAnalysis<I>::NodeState & newState, Instruction * ins, Instruction * otherIns){
        auto res = varMaps_.find(ins);
        auto operand = varMaps_.find(otherIns);
        if(operand != varMaps_.end() && res!=varMaps_.end()){
            auto operandLR = newState.find(operand->second);
            if(operand->second != res->second){
                if(operandLR != newState.end()) {
                    //live range of LHS merged, so it has to be removed from newState
                    // instead there will be extended/merged LR
                    newState.erase(operandLR);
                }
                auto newAddressLR = res->second->merge(operand->second);
                assert(newAddressLR == res->second);
                deletedLR_.emplace(operand->second, newAddressLR);
            }
            newState.emplace(res->second);
            varMaps_.insert_or_assign(otherIns, res->second);
            varMaps_.insert_or_assign(ins, res->second);
        }
    }


    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::transferFun(const CfgNode<I> *node, const ColoringLiveAnalysis::NodeState &state){
        if(dynamic_cast<const CfgFunExitNode<I> *>(node)){
            return nodeLattice_.bot();
        }else if (auto gext = dynamic_cast<const CfgGlobExitNode<I> *>(node)){

            return state;
        }else if(auto stmt = dynamic_cast<const CfgStmtNode<I> *>(node)){
            auto * stmtNode = dynamic_cast<const CfgStmtNode<I> *>(node);
            auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
            if(instr){ //TODO rules to add and remove from states


                if (dynamic_cast<AllocL *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
//                    return ;
                }else if (dynamic_cast<AllocG *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto store = dynamic_cast<Store *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
//                    auto valInstr  = varMaps_.find(store->value());
//                    if(valInstr != varMaps_.end()){
//                        newState.erase(valInstr->second);
//                    }else{
//                        throw "[LivenessAnalysis Target] cannot find value of instruction in live ranges";
//                    }

                    return newState;
                }else if (auto load = dynamic_cast<Load *>(stmtNode->il())){
                    auto newState = state;
//                    if(load->resultType() != ResultType::StructAddress){
                        std::set<CLiveRange*> children = getSubtree(node);
                        newState.insert(children.begin(), children.end());
                        auto res = varMaps_.find(node->il());
                        if(res!=varMaps_.end()){
                            auto r = newState.find(res->second);
                            if(r != newState.end()) {
                                newState.erase(r);
                            }
                        }
                        return newState;
//                    }else{
//                        std::set<CLiveRange*> children = getSubtree(node);
//                        newState.insert(children.begin(), children.end());
//                        combineLR(newState, load, load->address());
//                    }
                }else if (dynamic_cast<LoadImm *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
                }else if (auto ret =dynamic_cast<Return *>(stmtNode->il())){
                    auto newState = state;
                    auto res = varMaps_.find(ret->returnValue());
                    if(res != varMaps_.end()){
                        newState.emplace(res->second);
                    }
//                    auto res = varMaps_.find(node->il());
//                    if(res!=varMaps_.end()){
//                        newState.erase(newState.find(res->second));
//                    }
                    return newState;
                }else if (dynamic_cast<CondJump *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
//                    auto res = varMaps_.find(node->il());
//                    if(res!=varMaps_.end()){
//                        newState.erase(newState.find(res->second));
//                    }
                    return newState;
                }else if (dynamic_cast<Jump *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
//                    auto res = varMaps_.find(node->il());
//                    if(res!=varMaps_.end()){
//                        newState.erase(newState.find(res->second));
//                    }
                    return newState;
                }else if (dynamic_cast<PutChar *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<GetChar *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto copy = dynamic_cast<Copy *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto extend = dynamic_cast<Extend *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());

                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }

                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto trunc = dynamic_cast<Truncate *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());

                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }

                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto binop = dynamic_cast<BinOp *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    bool canCombine = canBinOpCombine(binop);
                    if(canCombine){
                        combineLR(newState, binop, binop->lhs());
                    }
//                    auto lhs = varMaps_.find(binop->lhs());
//                    auto res = varMaps_.find(binop);
//                    if(lhs != varMaps_.end() && res!=varMaps_.end() ){
//                        auto lhsLR = newState.find(lhs->second);
//                        if(lhs->second != res->second){
//                            if(lhsLR != newState.end()) {
//                                //live range of LHS merged, so it has to be removed from newState
//                                // instead there will be extended/merged LR
//                                newState.erase(lhsLR);
//                            }
//                            auto newAddressLR = res->second->merge(lhs->second);
//                            assert(newAddressLR == res->second);
//                            deletedLR_.emplace(lhs->second, newAddressLR);
//                        }
//                        newState.emplace(res->second);
//                        varMaps_.insert_or_assign(binop->lhs(), res->second);
//                        varMaps_.insert_or_assign(binop, res->second);
//                    }else{
//                        throw "[Liveness Analysis] cant find instruction in VarMaps";
//                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto unop = dynamic_cast<UnOp *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());

                    combineLR(newState, unop, unop->operand());
//                    auto operand = varMaps_.find(unop->operand());
//                    auto res = varMaps_.find(node->il());
//
//                    if(operand != varMaps_.end() &&res!=varMaps_.end()){
//                        auto operandLR = newState.find(operand->second);
//                        if(operand->second != res->second) {
//                            if (operandLR != newState.end()) {
//                                //live range of LHS merged, so it has to be removed from newState
//                                newState.erase(operandLR);
//                            }
//                            auto newAddressLR = res->second->merge(operand->second);
//                            assert(newAddressLR == res->second);
//                            deletedLR_.emplace(operand->second, newAddressLR);
//                        }
//                        varMaps_.insert_or_assign(unop->operand(), res->second);
//                        varMaps_.insert_or_assign(unop, res->second);
////                        auto r = newState.find(res->second);
////                        if(r != newState.end()) {
////                            newState.erase(r);
////                        }
//                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Return *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Phi *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Call *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<CallStatic *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrIndex *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrOffset *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<StructAssign *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }
            }


        } else{
        }
        return state;
    }




    template<class Info>
    CLiveVars<Info> ColoringLiveAnalysis<Info>::analyze() {
        CLiveVars<Info> X = lattice_.bot();
        std::set<const CfgNode<Info>*> W;
        for( auto & n : cfg_->nodes()){
            W.emplace(n);
        }

        while (!W.empty()) {
            const CfgNode<Info> * n = *W.begin();
            W.erase(W.begin());
            auto x = X.access(n);
            auto y = funOne(n, X);

            if (y != x) {
                X.update(std::make_pair(n, y));//X += n -> y
                //W ++= n.pred;
                W.insert( n->pred_.begin(), n->pred_.end());
            } else if (y.empty()) {
                X.update(std::make_pair(n, y));//X += n -> y;
            }
        }

        for (auto dlr : deletedLR_) {
            allocatedLR_.erase(dlr.first);

//            for (auto & x : X) {
//                auto foundDel= x.second.find(dlr.first);
//                if( foundDel != x.second.end()){
//                    x.second.erase(foundDel);
//                    x.second.emplace(dlr.second);
//                }
//            }
            delete dlr.first;
        }
        return std::move(X);
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::funOne(const CfgNode<I> * node, CLiveVars<I> & state)
    {
        return ColoringLiveAnalysis<I>::transferFun(node, join(node, state));
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::join(const CfgNode<I> * node, CLiveVars<I> & state)
    {
        auto acc = nodeLattice_.bot();
        for (const CfgNode<I> * s : node->succ_) {
            auto pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    template<typename Info>
    ColoringLiveAnalysis<Info>::ColoringLiveAnalysis( const Declarations &declarations, TargetProgram * program):
            BackwardAnalysis<CLiveVars<Info>, Info>(),
            varMaps_(),
            cfg_(this->getCfg(getProgram(program).get())),
            allVars_([&](){
                std::set< CLiveRange*> tmp;
                for ( auto & n : cfg_->nodes()){
                    if(auto t = dynamic_cast<Instruction *>(n->il()) ){
                        if(t->resultType() != ResultType::Void && !t->usages().empty()){
                            // instruction without usage does not need to be alive
                            auto lr = new CLiveRange(t, t);
                            allocatedLR_.emplace(lr);
                            tmp.emplace(lr);
                            varMaps_.emplace(n->il(), lr);
                        }
                    }
                }
                return tmp;
            }())
            ,
            nodeLattice_(CPowersetLattice<CLiveRange*>(allVars_)),
            lattice_(MapLattice<const CfgNode<Info>*, std::set<CLiveRange*>>(cfg_->nodes(), &nodeLattice_)),
    program_(program),
    declAnalysis_(getProgram(program_).get()){

    }
} //namespace tvlm
