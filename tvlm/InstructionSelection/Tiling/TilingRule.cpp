#include "TilingRule.h"

namespace tvlm{


bool DummyRule::matchesOn(CfgNode<TileInfo> * node) {
    return true;
}

bool SpecializedTilingRule::matchesOn(CfgNode<TileInfo> * node/*instruction*/) {
    if(opcode_ != Instruction::Opcode::Return /*Instruction opcode*/){
        for (int i = 0; i < childs_.size();i++ ) {
            childs_[i]->matchesOn(node->succ_[i]/*child on that offset*/);
        }
    }

    return false;
}

Instruction::Opcode SpecializedTilingRule::opcode() const {
    return opcode_;
}

//int SpecializedTilingRule::cost() const {
//    return cost_;
//}
}
