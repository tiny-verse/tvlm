#include "liveness_analysis.h"
#include "ILUsageVisitor.h"

tvlm::LivenessAnalysis tvlm::LivenessAnalysis::create(Program *p){
    auto analysis = InstructionAnalysis(p);
    Declarations decls = analysis.analyze();

    return LivenessAnalysis(getCfg(p), decls);
}

std::unordered_set<tvlm::Declaration> tvlm::LivenessAnalysis::getSubtree(const tvlm::CfgNode *node) {
    ILUsageVisitor v;
    v.begin(node->il());
    return v.result();
}

tvlm::LivenessAnalysis::NodeState
tvlm::LivenessAnalysis::transferFun(const tvlm::CfgNode *node, const tvlm::LivenessAnalysis::NodeState &state){
    if(dynamic_cast<const CfgFunExitNode *>(node)){
        return nodeLattice_.bot();
    }else if(dynamic_cast<const CfgStmtNode *>(node)){
        auto * stmtNode = dynamic_cast<const CfgStmtNode *>(node);
        auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
        if(instr){ //TODO rules to add and remove from states


            if (dynamic_cast<::tvlm::AllocL *>(stmtNode->il())){
                auto newState = state;
                std::unordered_set<Declaration> children = getSubtree(node);
                newState.insert(children.begin(), children.end());
                newState.erase(node->il());
                return newState;
            }else if (dynamic_cast<::tvlm::Store *>(stmtNode->il())){
                auto newState = state;
                std::unordered_set<Declaration> children = getSubtree(node);
                newState.insert(children.begin(), children.end());
                newState.erase(node->il());
                return newState;
            }else if (dynamic_cast<::tvlm::Load *>(stmtNode->il())){
                auto newState = state;
                std::unordered_set<Declaration> children = getSubtree(node);
                newState.insert(children.begin(), children.end());
                newState.erase(node->il());
                return newState;
            }else if (dynamic_cast<::tvlm::LoadImm *>(stmtNode->il())){
                auto newState = state;
                std::unordered_set<Declaration> children = getSubtree(node);
                newState.insert(children.begin(), children.end());
                newState.erase(node->il());
                return newState;
            }else if (dynamic_cast<::tvlm::Return *>(stmtNode->il())){
                auto ret  = dynamic_cast<::tvlm::Return *>(stmtNode->il());
                auto newState = state;
                newState.emplace(ret->returnValue());
                newState.erase(node->il());
                return newState;
            }else if (dynamic_cast<::tvlm::CondJump *>(stmtNode->il())){
                auto newState = state;
                std::unordered_set<Declaration> children = getSubtree(node);
                newState.insert(children.begin(), children.end());
                newState.erase(node->il());
                return newState;
            }else if (dynamic_cast<::tvlm::Jump *>(stmtNode->il())){
                auto newState = state;
                std::unordered_set<Declaration> children = getSubtree(node);
                newState.insert(children.begin(), children.end());
                newState.erase(node->il());
                return newState;
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

