#pragma once

#include <map>
#include <queue>
#include "tvlm/il_builder.h"
#include "tvlm/il_insns.h"
#include "instruction_analysis.h"
#include "cfg.h"

namespace tvlm{
    using Instruction = ::tvlm::Instruction;


using LiveVars = MAP<const CfgNode *, std::unordered_set<Declaration>>;
class LivenessAnalysis : public BackwardAnalysis<LiveVars>{
//    using Declaration = tvlm::Instruction*;
    using NodeState = std::unordered_set<Declaration>;
//    using Declarations = MAP< ILInstruction *, Declaration>;

    NodeState join(const CfgNode * node, LiveVars & state){
        auto states = node->succ_;
        auto acc = nodeLattice_.bot();
        for (const CfgNode * s : node->succ_) {
            auto & pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    LivenessAnalysis( ProgramCfg && cfg, const Declarations & declarations):
    allVars_([&](){
        std::unordered_set<Declaration> tmp;
        for ( auto & n : cfg.nodes()){
            tmp.emplace(n->il());
        }
        return tmp;
        }())
        ,
    nodeLattice_(PowersetLattice<Declaration>(allVars_)),
    lattice_(MapLattice<const CfgNode*, std::unordered_set<Declaration>>(cfg.nodes(), &nodeLattice_)),
    cfg_(std::move(cfg)){

    }

        std::unordered_set<Declaration> getSubtree(const CfgNode *pNode);

        NodeState transferFun(const CfgNode * node, const  NodeState & state );


    NodeState funOne(const CfgNode * node, LiveVars & state){
        return transferFun(node, join(node, state));
    }
public:
    static LivenessAnalysis * create(Program * p);
    LiveVars analyze() override{
        LiveVars X = lattice_.bot();
        std::unordered_set<const CfgNode*> W;
        for( auto & n : cfg_.nodes()){
            W.emplace(n);
        }

        while (!W.empty()) {
            const CfgNode* n = *W.begin();
            W.erase(W.begin());
            auto x = X.access(n);
            auto y = funOne(n, X);

            if (y != x) {
                X.insert(std::make_pair(n, y));//X += n -> y
                //W ++= n.pred;
                W.insert( n->pred_.begin(), n->pred_.end());
            } else if (y.empty()) {
                X.insert(std::make_pair(n, y));//X += n -> y;
            }
        }

        return X;
    }
private:
    NodeState allVars_;
    PowersetLattice<Declaration> nodeLattice_;
    MapLattice<const CfgNode *, NodeState> lattice_;
    ProgramCfg cfg_;

};

} //namespace tvlm
