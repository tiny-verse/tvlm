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
        std::set<CfgNode*> newExitNodes_;
        std::set<CfgNode*> newThatEntryNodes_;

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
    std::set<CfgNode*> newEntry;
    std::set<CfgNode*> newExit;
    std::set_union(that->entryNodes_.begin(), that->entryNodes_.end(), entryNodes_.begin(), entryNodes_.end(), std::inserter(newEntry, newEntry.begin())),
            std::set_union(that->entryNodes_.begin(), that->entryNodes_.end(), exitNodes_.begin(), exitNodes_.end(), std::inserter(newExit, newExit.begin()));

    return new FragmentCfg(newEntry, newExit);
}

std::set<const CfgNode*> FragmentCfg::nodes() {
    std::set<const CfgNode*> res;
    for (auto & e : entryNodes_) {
        res.merge(visit(e));
    }
    return res;
}

static std::set<const CfgNode*> visitHelper( const CfgNode * n, std::set<const CfgNode*> & visited){
    if(visited.find(n) != visited.end()){
        visited.insert(n);

        for( auto * next : n->succ_){
            visitHelper(next, visited);
        }
    }
    return visited;
}


std::set<const CfgNode*> FragmentCfg::visit(const CfgNode * n) {

    std::set<const CfgNode*> res;
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
