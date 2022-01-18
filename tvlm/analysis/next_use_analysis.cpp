#include "next_use_analysis.h"
#include "ILUsageVisitor.h"

tvlm::NextUseAnalysis * tvlm::NextUseAnalysis::create(Program *p){
    auto analysis = InstructionAnalysis(p);
    Declarations decls = analysis.analyze();

    return new NextUseAnalysis(getCfg(p), decls);
}

std::unordered_set<tvlm::Declaration*> tvlm::NextUseAnalysis::getSubtree(const tvlm::CfgNode *node) {
    ILUsageVisitor v;
    v.begin(node->il());
    return v.result();
}

tvlm::NextUseAnalysis::NodeState
tvlm::NextUseAnalysis::transferFun(const tvlm::CfgNode *node, const tvlm::NextUseAnalysis::NodeState &state){
    if(dynamic_cast<const CfgFunExitNode *>(node)){
        return nodeLattice_.bot();
    }else if(dynamic_cast<const CfgStmtNode *>(node)){
        auto * stmtNode = dynamic_cast<const CfgStmtNode *>(node);
        auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
        if(instr){ //TODO all instructions


            if (dynamic_cast<::tvlm::AllocL *>(stmtNode->il())){
//                auto alloc  = dynamic_cast<::tvlm::AllocL *>(stmtNode->il());
//                auto newState = state;
//                if(alloc)
//                std::unordered_set<Declaration*> children = getSubtree(node);
//                newState.insert(children.begin(), children.end());
//                newState.erase(node->il());
//                return newState;
            }else if (dynamic_cast<::tvlm::Store *>(stmtNode->il())){
                auto store  = dynamic_cast<::tvlm::Store *>(stmtNode->il());
                auto newState = state;

                newState[store->value()] = makeFlatVal(stmtNode->il(), newState);
                newState[store->address()] = makeFlatVal(stmtNode->il(), newState);
                return newState;
            }else if (dynamic_cast<::tvlm::Load *>(stmtNode->il())){
                auto load  = dynamic_cast<::tvlm::Load *>(stmtNode->il());
                auto newState = state;
                newState[load->address()] = makeFlatVal(stmtNode->il(), newState);
                return newState;


            }else if (dynamic_cast<::tvlm::LoadImm *>(stmtNode->il())){
                return state;
            }else if (dynamic_cast<::tvlm::Return *>(stmtNode->il())){
                auto ret  = dynamic_cast<::tvlm::Return *>(stmtNode->il());
                auto newState = state;

                newState[ret->returnValue()] = makeFlatVal(stmtNode->il(), newState);
                return newState;
            }else if (dynamic_cast<::tvlm::CondJump *>(stmtNode->il())){
            }else if (dynamic_cast<::tvlm::Jump *>(stmtNode->il())){
            }else {
                return state;
            }
        }else if (dynamic_cast<const CfgFunExitNode *>(node)){

            return nodeLattice_.bot();
        }


    } else{
    }
    return state;
}

