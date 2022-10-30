#pragma once

#include <utility>
#include <unordered_set>
#include <map>

#include "cfgNode.h"

using Function = tvlm::Function;
using Program = tvlm::Program;


namespace tvlm{

template<class T>
class FragmentCfg;

template<class T>
class FunctionCfg ;

template<class T>
class ProgramCfg ;

template<class T>
class GlobalCfg ;

template<class T>
class FragmentCfg {
public:
    virtual ~FragmentCfg() {};

    FragmentCfg(const std::unordered_set<CfgNode<T> *> &entryNodes, const std::unordered_set<CfgNode<T> *> &exitNodes) :
            entryNodes_(entryNodes),
            exitNodes_(exitNodes) {

    }

    FragmentCfg(const FragmentCfg<T> &other) : entryNodes_(other.entryNodes_),
                                               exitNodes_(other.exitNodes_) {}

    FragmentCfg(FragmentCfg<T> &&other) : entryNodes_(std::move(other.entryNodes_)),
                                               exitNodes_(std::move(other.exitNodes_)) {}

    bool isEmpty() const {
        return entryNodes_.empty() && exitNodes_.empty();
    }

    FragmentCfg<T> *concat(FragmentCfg<T> *that);

    FragmentCfg<T> *operator|(FragmentCfg<T> *that);

    std::unordered_set<const CfgNode<T> *> nodes();

protected:
    std::unordered_set<const CfgNode<T> *> visit(const CfgNode<T> *n);

    std::unordered_set<CfgNode<T> *> entryNodes_;
    std::unordered_set<CfgNode<T> *> exitNodes_;

private:
//    friend ProgramCfg<T> * get(Program * program, GlobalCfg<T> * globalCfg, std::unordered_set<FunctionCfg<T>*> & functionsCfg);
//template<class Tt>
    friend class ProgramCfg<T>;};

template<class T>
class FunctionCfg : public FragmentCfg<T>{
public:
    FunctionCfg( Function * fnc, CfgFunEntryNode<T> * entry, CfgFunExitNode<T> * exit, FragmentCfg<T> *  cfg = nullptr) :
            FragmentCfg<T>([&]( CfgFunEntryNode<T> * ent) {
                                    std::unordered_set<CfgNode<T>*> res;
                                    res.insert(ent);
                                    return res;
                                }(entry),
                           [&]( CfgFunExitNode<T> * ext) {
                                    std::unordered_set<CfgNode<T>*> res;
                                    res.insert(ext);
                                    return res;
                                }(exit)
                           ),
            entry_(entry),
            exit_(exit),
            cfg_(cfg){

    }
    virtual ~FunctionCfg(){
        for (auto *n : nodes_) {
            delete n;
        }
        for (auto* n : frags_) {
            delete n;
        }
//        for (int i = 0; i < frags_.size(); ++i) {
//            delete frags_[i];
//        }
    }
    const CfgFunEntryNode<T> * entry()const {
        return entry_;
    }
    const CfgFunExitNode<T> * exit()const {
        return exit_;
    }
    CfgNode<T> * addNode(CfgNode<T> * node){
        nodes_.push_back((node));
        return node;
    }
    FragmentCfg<T> * addFragment(FragmentCfg<T> * f){
        frags_.push_back(f);
        return f;
    }
    void setCfg(FragmentCfg<T> * cfg){
        cfg_ = cfg;
    }
private:
    std::vector<CfgNode<T> *> nodes_;
    std::vector<FragmentCfg<T> *> frags_;
    CfgFunEntryNode<T> *entry_;
    CfgFunExitNode<T> * exit_;
    FragmentCfg<T> * cfg_;
};

template<class T>
class GlobalCfg : public FunctionCfg<T>{
public:
    GlobalCfg( BasicBlock * fnc, CfgFunEntryNode<T> * entry, CfgFunExitNode<T> * exit, FragmentCfg<T> *  cfg = nullptr) :
            FunctionCfg<T>(nullptr, entry, exit, cfg) {

    }
    virtual ~GlobalCfg() = default;
};

template<class T>
class ProgramCfg : public FragmentCfg<T>{
protected:
    ProgramCfg(Program * program, std::unordered_set<CfgNode<T>*> && entry,
               std::unordered_set<CfgNode<T>*> && exit,
               std::vector<std::unique_ptr<FragmentCfg<T>>> && functionsCfg):
            FragmentCfg<T>(std::move(entry), std::move(exit)),
            functionsCfg_(std::move(functionsCfg)),
            p_(program)
            {

    }

public:
    virtual ~ProgramCfg() = default;
    ProgramCfg(ProgramCfg && other): FragmentCfg<T>(std::move(other.entryNodes_), std::move(other.exitNodes_)),
    functionsCfg_(std::move(other.functionsCfg_) ),
    p_(other.p_){

    }

    ProgramCfg(const ProgramCfg<T> & other) = delete;
//    :FragmentCfg(other.entryNodes_, other.exitNodes_),
//    p_(other.p_){
//        functionsCfg_.reserve(other.functionsCfg_.size());
//        auto e  = other.functionsCfg_.begin();
//        auto f = functionsCfg_.begin();
//        for (; e != other.functionsCfg_.end();e++, f++) {
//            functionsCfg_.emplace_back(new FunctionCfg(**(e)));
//        }
//    }

public:

    static ProgramCfg<T> * get(Program * program, GlobalCfg<T> * globalCfg, std::unordered_set<FunctionCfg<T>*> & functionsCfg);


private:
    std::vector<std::unique_ptr<FragmentCfg<T>>> functionsCfg_;
    Program  * p_;

};

using ILInstruction = Instruction;

template<class T>
class CfgBuilder {
public:
    virtual FragmentCfg<T>* fromInstruction( ILInstruction* ins ) = 0;

    virtual GlobalCfg<T> * fromGlobals(BasicBlock * bb) = 0;

    virtual FunctionCfg<T>* fromFunction( Function * fnc) = 0;

    ProgramCfg<T> * fromProgram(Program * p){
        CfgNode<T>::counter_ = 0;
        std::unordered_set<FunctionCfg<T>*> tmp;
        for (auto & ff : p->functions()) {
            tmp.insert( fromFunction(ff.second.get()) );
        }


        return ProgramCfg<T>::get(p,fromGlobals(p->globals_.get()), tmp);
    }

protected:
    static std::vector<std::unique_ptr<BasicBlock>> & getBBs(Function * fnc){
        return fnc->bbs_;
    }

    static std::vector<std::unique_ptr<Instruction>> & getInstructions(BasicBlock * bb) {
        return bb->insns_;
    }
};

template<class T>
class IntraProceduralCfgBuilder : public CfgBuilder<T>{
public:
    FragmentCfg<T> * fromBB(BasicBlock * bb);

    GlobalCfg<T> * fromGlobals(BasicBlock * bb);

    FragmentCfg<T> * fromInstruction(ILInstruction * ins);

    FunctionCfg<T> * fromFunction(Function * fnc);


private:
    CfgNode<T> * makeStmtNode(ILInstruction * il){
        auto it = allNodesMap_.find(il);
        if(it != allNodesMap_.end()){
            return it->second;
        }
        return allNodesMap_[il] = append(new CfgStmtNode<T>(il));
    }


    CfgNode<T> * append(CfgNode<T> * a){
//        allNodes.emplace_back(a);
        return current_fnc->addNode(a);
    }

    FragmentCfg<T> * append(FragmentCfg<T> * a){
//        allFragments.emplace_back(a);
        return current_fnc->addFragment(a);
    }

//    std::vector<std::unique_ptr<CfgNode<T>>> allNodes;
//    std::vector<std::unique_ptr<FragmentCfg<T>>> allFragments;
    std::map<IL*, CfgNode<T>*> allNodesMap_;
    std::map<IL*, FragmentCfg<T>*> allFragmentsMap_;
    FragmentCfg<T> * fncexit_;
    FunctionCfg<T> * current_fnc;

    FragmentCfg<T> * empty(){
        return new FragmentCfg(std::unordered_set<CfgNode<T>*> (), std::unordered_set<CfgNode<T>*> ());
    }
    FragmentCfg<T> * single(CfgNode<T> * node ){
        std::unordered_set<CfgNode<T> *> a;
        a.insert(node);
        return append(new FragmentCfg(a, a));
    }
    //liveness for registers
};


    template<class T>
    FragmentCfg<T> * FragmentCfg<T>::concat(FragmentCfg * that) {
        if(this->isEmpty()){
            return new FragmentCfg(*that);
        }else if ( that->isEmpty()){
            return new FragmentCfg(*this);
        }else {
            std::unordered_set<CfgNode<T>*> newExitNodes_;
            std::unordered_set<CfgNode<T>*> newThatEntryNodes_;

            for ( CfgNode<T> *ex: exitNodes_) {
                ex->succ_.insert(ex->succ_.end(), that->entryNodes_.begin(), that->entryNodes_.end());
                newExitNodes_.insert(ex);
            }
            exitNodes_ = newExitNodes_;

            for (CfgNode<T> *en :that->entryNodes_) {
                en->pred_.insert(en->pred_.end(), exitNodes_.begin(), exitNodes_.end());
                newThatEntryNodes_.insert(en);
            }
            that->entryNodes_ = newThatEntryNodes_;

            return new FragmentCfg<T>(entryNodes_, that->exitNodes_);
        }

    }
    template<class T>
    FragmentCfg<T> * FragmentCfg<T>::operator|(FragmentCfg<T> * that) {
        std::unordered_set<CfgNode<T>*> newEntry;
        std::unordered_set<CfgNode<T>*> newExit;
        std::set_union(that->entryNodes_.begin(), that->entryNodes_.end(), entryNodes_.begin(), entryNodes_.end(), std::inserter(newEntry, newEntry.begin())),
                std::set_union(that->entryNodes_.begin(), that->entryNodes_.end(), exitNodes_.begin(), exitNodes_.end(), std::inserter(newExit, newExit.begin()));

        return new FragmentCfg<T>(newEntry, newExit);
    }

    template<class T>
    std::unordered_set<const CfgNode<T>*> FragmentCfg<T>::nodes() {
        std::unordered_set<const CfgNode<T>*> res;
        for (auto & e : entryNodes_) {
            res.merge(visit(e));
        }
        return res;
    }

    template<class T>
    static std::unordered_set<const CfgNode<T>*> visitHelper( const CfgNode<T> * n, std::unordered_set<const CfgNode<T>*> & visited){
        if(visited.find(n) == visited.end()){
            visited.insert(n);

            for( auto * next : n->succ_){
                visitHelper(next, visited);
            }
        }
        return visited;
    }

    template<class T>
    std::unordered_set<const CfgNode<T>*> FragmentCfg<T>::visit(const CfgNode<T> * n) {

        std::unordered_set<const CfgNode<T>*> res;
        visitHelper(n,res);
        return res;
    }

    template<class T>
    FragmentCfg<T> * IntraProceduralCfgBuilder<T>::fromBB(::tvlm::BasicBlock *bb) {

        auto it = allFragmentsMap_.find(bb);
        if( it != allFragmentsMap_.end()){
            return it->second;
        }
        auto acc = empty();
        allFragmentsMap_[bb] = acc;
        for ( auto & ins : CfgBuilder<T>::getInstructions(bb)) {
            auto tmp = acc->concat(fromInstruction(ins.get()));
            delete acc;
            allFragmentsMap_[bb] = tmp;
            acc = tmp;
        }

        return allFragmentsMap_[bb] = acc;
    }

    template<class T>
    GlobalCfg<T> * IntraProceduralCfgBuilder<T>::fromGlobals(::tvlm::BasicBlock *bb) {


        auto entry  = new CfgFunEntryNode<T>(bb);
        auto exit  = new CfgFunExitNode<T>(bb);
        auto globalCfg = new GlobalCfg(bb, entry, exit);
        current_fnc = globalCfg;
//        globalCfg->addNode(entry);
        globalCfg->addNode(exit);
        auto entryPtr = single(entry);
        auto exitPtr = single(exit);


        auto acc = empty();
        for ( auto & ins : CfgBuilder<T>::getInstructions(bb)) {
            auto tmp = acc->concat(fromInstruction(ins.get()));
            delete acc;
            acc = tmp;
        }

        FragmentCfg<T> * fullGlobalCfg = entryPtr->concat(acc)->concat(exitPtr);
//        globalCfg->addNode(fullGlobalCfg); //TODO how to do better memory management
        globalCfg->addFragment(fullGlobalCfg);
        globalCfg->setCfg(fullGlobalCfg);
        current_fnc = nullptr;
        return globalCfg;
    }

    template<class T>
    FunctionCfg<T> * IntraProceduralCfgBuilder<T>::fromFunction(Function *fnc) {
        auto entry  = new CfgFunEntryNode<T>(fnc);
        auto exit = new CfgFunExitNode<T>(fnc);
        auto fncCfg = new FunctionCfg(fnc, entry, exit);
        current_fnc = fncCfg;
        append(entry);
        append(exit);
        auto ptr = single(entry);
        fncexit_ = single(exit);
        auto blockCfg = fromBB(CfgBuilder<T>::getBBs(fnc)[0].get());
        FragmentCfg<T> * cfg = ptr->concat(blockCfg)->concat(fncexit_);
        append(cfg);
//        for (auto & e: allNodes) {
//            fncCfg->addNode(e.release());
//        }allNodes.clear();
//        for (auto & e: allFragments) {
//            fncCfg->addFragment(e.release());
//            assert(e == nullptr);
//        }allFragments.clear();
        current_fnc = nullptr;
        fncCfg->setCfg(cfg);
        return fncCfg;
    }

    template<class T>
    FragmentCfg<T> *tvlm::IntraProceduralCfgBuilder<T>::fromInstruction(tvlm::ILInstruction *ins) {

        if(dynamic_cast<::tvlm::CondJump *>(ins)) {
            ::tvlm::CondJump * condJump = dynamic_cast<::tvlm::CondJump*>(ins);
            auto guardCfg = single(makeStmtNode(condJump));
            auto guardedThenCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(1)))));
            if(condJump->numTargets() == 2){
                auto guardedElseCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(0)))));
                return append(*guardedThenCfg | guardedElseCfg);
            }
            return append( *guardedThenCfg | guardCfg);
        } else if (dynamic_cast<::tvlm::Jump *>(ins)){
            auto * jmp = dynamic_cast<::tvlm::Jump*>(ins);
            auto jmpFrag = single(makeStmtNode(jmp));

            auto dest  = fromBB(jmp->getTarget(1));
            return append(jmpFrag->concat(dest));
        }else if (dynamic_cast<::tvlm::Return *>(ins)){
            auto * ret = dynamic_cast<::tvlm::Return*>(ins);
            auto retFrag = single(makeStmtNode(ret));
            auto dest  = fncexit_;
            return append(retFrag->concat(dest));
        }

        return single(makeStmtNode(ins));
    }
    template<class T>
    ProgramCfg<T>* ProgramCfg<T>::get(Program *program, GlobalCfg<T> * globalCfg, std::unordered_set<FunctionCfg<T> *> &functionsCfg) {
        std::unordered_set<CfgNode<T>*> setEntry;
        std::unordered_set<CfgNode<T>*> setExit;
        std::vector<std::unique_ptr<FragmentCfg<T>>> tmp;
//        FragmentCfg<T> *  AllGlobals = new FragmentCfg(std::unordered_set<CfgNode<T>*> (), std::unordered_set<CfgNode<T>*> ());

        tmp.reserve(functionsCfg.size());
        for(auto & e: functionsCfg ){
            auto prependedGlobals = globalCfg->concat (e);
            auto uPtr = std::unique_ptr<FragmentCfg<T>>(prependedGlobals);
            tmp.emplace_back(std::move(uPtr));
//            auto tmpAllGlobals = AllGlobals;
//            AllGlobals = (AllGlobals->concat( prependedGlobals));
//            delete tmpAllGlobals;

            setExit.insert((CfgNode<T>*)e->exit());
            tmp.emplace_back(e);
        }
//        AllGlobals = globalCfg->concat ((CfgNode<T>*)e->entry());
//        tmp.emplace_back(AllGlobals);

//        for(auto * i : ProgramFrag->entryNodes_){
//            setEntry.insert(i);
//        }
        for(auto * i : globalCfg->entryNodes_){
            setEntry.insert(i);
        }
//        setEntry.insert(globalCfg);
        return new ProgramCfg(program, std::move(setEntry), std::move(setExit) , std::move(tmp) );
    }
//ProgramCfg<T>* ProgramCfg<T>::get(Program *program, FragmentCfg<T> * globalCfg, std::unordered_set<FunctionCfg<T> *> &functionsCfg) {
//        std::unordered_set<CfgNode<T>*> setEntry;
//        for(auto & e: functionsCfg ){
//            setEntry.insert((CfgNode<T>*)e->entry());
//        }
//        std::unordered_set<CfgNode<T>*> setExit;
//        for(auto & e: functionsCfg ){
//            setExit.insert((CfgNode<T>*)e->exit());
//        }
//
//        std::vector<std::unique_ptr<FunctionCfg<T>>> tmp;
//        std::vector<std::unique_ptr<CfgNode<T>>> globalNodes;
//
//
//        tmp.reserve(functionsCfg.size());
//        for (const auto e: functionsCfg) {
//            tmp.emplace_back(e);
//        }
//        return new ProgramCfg(program, std::move(setEntry), globalCfg, std::move(setExit) , std::move(tmp) );
//    }

//    template<class T>
//    FunctionCfg<T>::FunctionCfg(FunctionCfg &&fnc) : FragmentCfg<T>(std::move(fnc.entryNodes_), std::move(fnc.exitNodes_) ),
//                                                     entry_(fnc.entry_), exit_(fnc.exit_), cfg_(fnc.cfg_),
//                                                     nodes_(std::move(fnc.nodes_)),frags_(std::move(fnc.frags_)) {
//        fnc.entry_ = nullptr;
//        fnc.exit_ = nullptr;
//        fnc.cfg_ = nullptr;
//        fnc.nodes_.clear();
//        fnc.frags_.clear();
//    }

} // namespace tiny::tvlm
