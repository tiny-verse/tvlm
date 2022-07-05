#pragma once

#include "tvlm/il/il.h"
#include "tvlm/analysis/cfgNode.h"


namespace tvlm{


    /**
     * Rule Should be able to test if it can be matched on expression
     *
     * */

    class TileInfo{
        std::vector<TilingRule * > possibleRules_;
        TilingRule * selectedRule_;
    };

    class TilingRule{
    public:
        TilingRule(int cost = 1):cost_(cost){}

        virtual bool matchesOn(CfgNode<TileInfo> * node /*part of DAG*/) = 0;

        virtual Instruction::Opcode opcode()const{
            throw "no opcode";
        }
        int cost()const {
            return cost_;
        };
        int cost_;
    };

    class SpecializedTilingRule : public TilingRule {
    public:
        SpecializedTilingRule(bool commutative = false):commutative_(commutative){

        }
        bool matchesOn(CfgNode<TileInfo> * node) override;

        Instruction::Opcode opcode() const override;


    private:
        bool commutative_;
        Instruction::Opcode opcode_;
        std::vector<std::unique_ptr<TilingRule>> childs_;

    };

    class DummyRule : public TilingRule{
    public:
        DummyRule():TilingRule(0){}
        bool matchesOn(CfgNode<TileInfo> * node) override;
    };
}
