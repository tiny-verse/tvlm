#pragma once

#include <map>
#include <set>
#include "tvlm/il_builder.h"
#include "tvlm/il_insns.h"
#include "instruction_analysis.h"

namespace tvlm{
    using Instruction = ::tvlm::Instruction;


using LiveVars = MAP<const CfgNode *, std::set<Declaration>>;
class LivenessAnalysis : public BackwardAnalysis<LiveVars>{
//    using Declaration = tvlm::Instruction*;
    using NodeState = std::set<Declaration>;
//    using Declarations = MAP< ILInstruction *, Declaration>;

    NodeState join(const CfgNode * node, LiveVars & state){
        auto states = node->succ_;
        auto acc = nodeLattice_.bot();
        for (const CfgNode * s : node->succ_) {
            auto pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    LivenessAnalysis( ProgramCfg  cfg, Declarations & declarations):
    allVars_(),
    nodeLattice_(PowersetLattice<Declaration>(allVars_)),
    lattice_(MapLattice<const CfgNode*, std::set<Declaration>>(cfg.nodes(), &nodeLattice_)),
    cfg_(cfg){

    }

    static LivenessAnalysis create(Program * p);
    NodeState transferFun(const CfgNode * node,const  NodeState & state ){
        if(dynamic_cast<const CfgFunExitNode *>(node)){
            return nodeLattice_.bot();
        }else if(dynamic_cast<const CfgFunEntryNode *>(node)){
            auto * stmtNode = dynamic_cast<const CfgStmtNode *>(node);
            auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
            if(instr){ //TODO rules to add and remove from states


//            }else if (dynamic_cast<::tvlm:: *>(stmtNode->il())){
                return state; // WRONG
            }else {
                return state;
            }


        } else{
            return state;
        }
    }

    NodeState funOne(const CfgNode * node, LiveVars & state){
        return transferFun(node, join(node, state));
    }

    LiveVars analyze() override{
        LiveVars X = lattice_.bot();
        auto W = cfg_.nodes();

        while (!W.empty()) {
            const CfgNode* n = *W.begin();
            W.erase(n);
            auto x = X.access(n);
            auto y = funOne(n, X);

            if (y != x) {
                X.insert(make_pair(n, y));//X += n -> y
                W.insert(n->pred_.begin(), n->pred_.end()); //W ++= n.pred;
            } else if (y.empty()) {
                X.insert(std::make_pair(n, y));//X += n -> y;
            }
        }

        return X;
    }
private:
    std::set<Declaration> allVars_;
    PowersetLattice<Declaration> nodeLattice_;
    MapLattice<const CfgNode *, std::set<Declaration>> lattice_;
    ProgramCfg cfg_;

};

} //namespace tvlm
