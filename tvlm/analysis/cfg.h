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
    }
    const CfgFunEntryNode<T> * entry()const {
        return entry_;
    }
    const CfgFunExitNode<T> * exit()const {
        return exit_;
    }
    CfgNode<T> * addNode(CfgNode<T> * node){
        nodes_.emplace(node);
        return node;
    }
    FragmentCfg<T> * addFragment(FragmentCfg<T> * f){
        frags_.emplace(f);
        return f;
    }
    void setCfg(FragmentCfg<T> * cfg){
        cfg_ = cfg;
    }
private:
    std::set<CfgNode<T> *> nodes_;
    std::set<FragmentCfg<T> *> frags_;
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
public:

    static ProgramCfg<T> * get(Program * program, GlobalCfg<T> * globalCfg, std::unordered_set<FunctionCfg<T>*> & functionsCfg);

private:
    std::vector<std::unique_ptr<FragmentCfg<T>>> functionsCfg_;
    Program  * p_;

};

using ILInstruction = Instruction;

template<class T>
class CfgBuilder : public ILFriend{
public:
    CfgBuilder(std::map<const CfgNode<T>*, const Instruction *> * cfg_mapping,
               std::map<const Instruction *, const CfgNode<T>*> * instr_mapping):
    cfg_mapping_(cfg_mapping),
    instr_mapping_(instr_mapping)
    {
    }

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
        return getpureBBsInstructions(bb);
    }
    std::map<const CfgNode<T>*, const Instruction *> * cfg_mapping_;
    std::map<const Instruction *, const CfgNode<T>*> * instr_mapping_;

};

template<class T>
class IntraProceduralCfgBuilder : public CfgBuilder<T>{
public:
    IntraProceduralCfgBuilder(std::map<const CfgNode<T>*, const Instruction *> * cfg_mapping,
                              std::map<const Instruction *, const CfgNode<T>*> * instr_mapping)
    :
    CfgBuilder<T>(cfg_mapping, instr_mapping)
    {

    }

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
        allNodesMap_[il] = append(new CfgStmtNode<T>(il));
        if(auto instr =dynamic_cast<ILInstruction * >(il)){
            this->cfg_mapping_->insert_or_assign(allNodesMap_[il], instr);
            this->instr_mapping_->insert_or_assign(instr, allNodesMap_[il]);
        }
        return allNodesMap_[il];
    }


    CfgNode<T> * append(CfgNode<T> * a){
        return current_fnc->addNode(a);
    }

    FragmentCfg<T> * append(FragmentCfg<T> * a){
        return current_fnc->addFragment(a);
    }


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
            auto tmp = visit(e);
            res.insert(tmp.begin(), tmp.end());
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
        return std::move(res);
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
        auto exit  = new CfgGlobExitNode<T>(bb);
        auto globalCfg = new GlobalCfg(bb, entry, exit);
        current_fnc = globalCfg;
//        globalCfg->addNode(entry);
//        globalCfg->addNode(exit);
        auto entryPtr = single(entry);
        auto exitPtr = single(exit);


        auto acc = empty();
        for ( auto & ins : CfgBuilder<T>::getInstructions(bb)) {
            auto tmp = acc->concat(fromInstruction(ins.get()));
            delete acc;
            acc = tmp;
        }

        FragmentCfg<T> * fullGlobalCfg = entryPtr->concat(acc)->concat(exitPtr);

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
        FragmentCfg<T> * cfg = ptr->concat(blockCfg); // exit appended by return
        append(cfg);

        current_fnc = nullptr;
        fncCfg->setCfg(cfg);
        return fncCfg;
    }

    template<class T>
    FragmentCfg<T> *tvlm::IntraProceduralCfgBuilder<T>::fromInstruction(tvlm::ILInstruction *ins) {

        if(auto condJump = dynamic_cast<::tvlm::CondJump *>(ins)) {
            auto guardCfg = single(makeStmtNode(condJump));
            auto guardedThenCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(1)))));
            if(condJump->numTargets() == 2){
                auto guardedElseCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(0)))));
                return append(*guardedThenCfg | guardedElseCfg);
            }
            return append( *guardedThenCfg | guardCfg);
        } else if (auto jmp = dynamic_cast<::tvlm::Jump *>(ins)){
            auto jmpFrag = single(makeStmtNode(jmp));

            auto dest  = fromBB(jmp->getTarget(1));
            return append(jmpFrag->concat(dest));
        }else if (auto ret = dynamic_cast<::tvlm::Return *>(ins)){
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

        tmp.reserve(functionsCfg.size());
        for(auto & e: functionsCfg ){
            auto prependedGlobals = globalCfg->concat (e);
            auto uPtr = std::unique_ptr<FragmentCfg<T>>(prependedGlobals);
            tmp.emplace_back(std::move(uPtr));

            setExit.insert((CfgNode<T>*)e->exit());
            tmp.emplace_back(e);
        }

        for(auto * i : globalCfg->entryNodes_){
            setEntry.insert(i);
        }

        return new ProgramCfg(program, std::move(setEntry), std::move(setExit) , std::move(tmp) );
    }

} // namespace tiny::tvlm
