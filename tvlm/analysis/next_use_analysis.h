#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "lattice/lattice.h"
#include "lattice/flat_lattice.h"

#include "cfg.h"
#include "tvlm/analysis/lattice/next_use_lattice.h"

namespace tvlm{
    using Instruction = ::tvlm::Instruction;


using NextUse = MAP<const CfgNode* ,  MAP<Declaration*, FlatElem<const Declaration*>*>>;
class NextUseAnalysis : public BackwardAnalysis<NextUse>{
//    using Declaration = tvlm::Instruction*;
    using NodeState =  MAP<Declaration*,FlatElem<const Declaration*>*>;
//    using NodeState = MAP<const CfgNode * , Declaration*>;
//    using Declarations = MAP< ILInstruction *, Declaration*>;

    NodeState join(const CfgNode * node, NextUse & state){
        auto states = node->succ_;
        auto acc = nodeLattice_.bot();
        for (const CfgNode * s : node->succ_) {
            auto & pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    NextUseAnalysis( ProgramCfg && cfg, const Declarations & declarations):
    allVars_([&](){
        std::unordered_set<Declaration*> tmp;
        for ( auto & n : cfg.nodes()){
             tmp.emplace(n->il());
        }
        return tmp;
        }())
        ,
    sublattice(FlatLattice<const Declaration *>()),
    nodeLattice_(NextUseLattice(allVars_, &sublattice )),
    lattice_(MapLattice<const CfgNode*, NodeState>(cfg.nodes(), &nodeLattice_)),
    cfg_(std::move(cfg)){

    }

        std::unordered_set<Declaration*> getSubtree(const CfgNode *pNode);

        NodeState transferFun(const CfgNode * node, const  NodeState & state );


    NodeState funOne(const CfgNode * node, NextUse & state){
        return transferFun(node, join(node, state));
    }
public:
    static NextUseAnalysis * create(Program * p);
    NextUse analyze() override{
        NextUse X = lattice_.bot();
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
            }
//            else if (y()) {
//                X.insert(std::make_pair(n, y));//X += n -> y;
//            }
        }

        return X;
    }
private:
    std::unordered_set<Declaration *> allVars_;
    FlatLattice<const Declaration*>sublattice;
//    MapLattice<Declaration , FlatElem<const Declaration>*> nodeLattice_;
    NextUseLattice nodeLattice_;
    MapLattice<const CfgNode *, NodeState> lattice_;
    ProgramCfg cfg_;
//    std::unordered_set<Declaration> allVars_;
//    MapLattice<const CfgNode*, Declaration> nodeLattice_;
//    MapLattice<const CfgNode *, NodeState> lattice_;
//    ProgramCfg cfg_;
//    std::vector<std::unique_ptr<FlatVal<const Declaration >>> createdFlatVals_;

    FlatVal<const Declaration*> * makeFlatVal(const Declaration * val , NodeState & nodeState) {
        auto tmp = new FlatVal<const Declaration*> (val);
        nodeState.reg(tmp);

        //        createdFlatVals_.emplace_back(tmp);
        return tmp;
    }
};

} //namespace tvlm
