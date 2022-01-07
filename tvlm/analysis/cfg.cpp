#include "cfg.h"
#include "cfgNode.h"

using FragmentCfg = tvlm::FragmentCfg;
using CfgNode = tvlm::CfgNode;

FragmentCfg * FragmentCfg::concat(FragmentCfg * that) {
    if(isEmpty()){
        return that;
    }else if ( that->isEmpty()){
        return this;
    }else {
        std::unordered_set<CfgNode*> newExitNodes_;
        std::unordered_set<CfgNode*> newThatEntryNodes_;

        for ( CfgNode *ex: exitNodes_) {
            ex->succ_.insert(that->entryNodes_.begin(), that->entryNodes_.end());
            newExitNodes_.insert(ex);
        }
        exitNodes_ = newExitNodes_;

        for (CfgNode *en :that->entryNodes_) {
            en->pred_.insert(exitNodes_.begin(), exitNodes_.end());
            newThatEntryNodes_.insert(en);
        }
        that->entryNodes_ = newThatEntryNodes_;

        return new FragmentCfg(entryNodes_, that->exitNodes_);
    }

}

FragmentCfg * FragmentCfg::operator|(FragmentCfg * that) {
    std::unordered_set<CfgNode*> newEntry;
    std::unordered_set<CfgNode*> newExit;
    std::set_union(that->entryNodes_.begin(), that->entryNodes_.end(), entryNodes_.begin(), entryNodes_.end(), std::inserter(newEntry, newEntry.begin())),
            std::set_union(that->entryNodes_.begin(), that->entryNodes_.end(), exitNodes_.begin(), exitNodes_.end(), std::inserter(newExit, newExit.begin()));

    return new FragmentCfg(newEntry, newExit);
}

std::unordered_set<const CfgNode*> FragmentCfg::nodes() {
    std::unordered_set<const CfgNode*> res;
    for (auto & e : entryNodes_) {
        res.merge(visit(e));
    }
    return res;
}

static std::unordered_set<const CfgNode*> visitHelper( const CfgNode * n, std::unordered_set<const CfgNode*> & visited){
    if(visited.find(n) == visited.end()){
        visited.insert(n);

        for( auto * next : n->succ_){
            visitHelper(next, visited);
        }
    }
    return visited;
}


std::unordered_set<const CfgNode*> FragmentCfg::visit(const CfgNode * n) {

    std::unordered_set<const CfgNode*> res;
    visitHelper(n,res);
    return res;
}

FragmentCfg *tvlm::IntraProceduralCfgBuilder::fromBB(::tvlm::BasicBlock *bb) {
    auto acc = std::unique_ptr<FragmentCfg>(empty());
    for ( auto & ins : getInstructions(bb)) {
        acc.reset( acc->concat(fromInstruction(ins.get())));
    }
    return acc.release();
}

tvlm::FunctionCfg *tvlm::IntraProceduralCfgBuilder::fromFunction(Function *fnc) {
    auto entry  = new CfgFunEntryNode(fnc);
    append(entry);
    auto blockCfg = fromBB(getBBs(fnc)[0].get());
    auto exit = new CfgFunExitNode(fnc);
    append(exit);
    auto ptr = single(entry);
    auto ptr2 = ptr->concat(blockCfg);
    auto cfg = ptr2->concat(single(exit));

    return new FunctionCfg(fnc, entry, exit, cfg);
}

FragmentCfg *tvlm::IntraProceduralCfgBuilder::fromInstruction(tvlm::ILInstruction *ins) {
//        if(dynamic_cast<::tvlm::Load *>(ins)) {//assignment // TODO
//            return single(new CfgStmtNode(ins));
//        }else if (){
//
//        }

    if(dynamic_cast<::tvlm::CondJump *>(ins)) {
        ::tvlm::CondJump * condJump = dynamic_cast<::tvlm::CondJump*>(ins);
        auto guardCfg = single(append(new CfgStmtNode(condJump->condition())));
        auto guardedThenCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(1)))));
        if(condJump->numTargets() == 2){
            auto guardedElseCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(0)))));
            return append(*guardedThenCfg | guardedElseCfg);
        }
        return append( *guardedThenCfg | guardCfg);
    }

    return single(append(new CfgStmtNode(ins)));
}

tvlm::ProgramCfg tvlm::ProgramCfg::get(Program *program, std::unordered_set<FunctionCfg *> &functionsCfg) {
    std::unordered_set<CfgNode*> setEntry;
    for(auto & e: functionsCfg ){
        setEntry.insert((CfgNode*)e->entry());
    }
    std::unordered_set<CfgNode*> setExit;
    for(auto & e: functionsCfg ){
        setExit.insert((CfgNode*)e->exit());
    }

    std::vector<std::unique_ptr<FunctionCfg>> tmp;
    tmp.reserve(functionsCfg.size());
    for (const auto e: functionsCfg) {
        tmp.emplace_back(e);
    }
    return ProgramCfg(program, std::move(setEntry) ,std::move(setExit) , std::move(tmp) );
}
