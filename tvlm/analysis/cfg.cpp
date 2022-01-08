#include "cfg.h"
#include "cfgNode.h"

using FragmentCfg = tvlm::FragmentCfg;
using CfgNode = tvlm::CfgNode;

FragmentCfg * FragmentCfg::concat(FragmentCfg * that) {
    if(isEmpty()){
        return new FragmentCfg(*that);
    }else if ( that->isEmpty()){
        return new FragmentCfg(*this);
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

    auto it = allFragmentsMap_.find(bb);
    if( it != allFragmentsMap_.end()){
        return it->second;
    }
    auto acc = std::unique_ptr<FragmentCfg>(empty());
    allFragmentsMap_[bb] = acc.get();
    for ( auto & ins : getInstructions(bb)) {
        acc.reset( acc->concat(fromInstruction(ins.get())));
        allFragmentsMap_[bb] = acc.get();

    }

    return allFragmentsMap_[bb] = acc.release();
}

tvlm::FunctionCfg *tvlm::IntraProceduralCfgBuilder::fromFunction(Function *fnc) {
    auto entry  = new CfgFunEntryNode(fnc);
    append(entry);
    auto ptr = single(entry);
    FragmentCfg * cfg;
    auto exit = new CfgFunExitNode(fnc);
    fncexit_ = single(append(exit));
    auto blockCfg = fromBB(getBBs(fnc)[0].get());
    cfg = append(ptr->concat(blockCfg));

    return new FunctionCfg(fnc, entry, exit, cfg);
}

FragmentCfg *tvlm::IntraProceduralCfgBuilder::fromInstruction(tvlm::ILInstruction *ins) {

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
