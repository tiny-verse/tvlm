#pragma once
#include <set>
#include "../il.h"

using Instruction = ::tvlm::Instruction;
using Function  = ::tvlm::Function;


    namespace tvlm {

        class CfgNode {
        public:
            int id_;
            std::set<CfgNode*> pred_;
            std::set<CfgNode*> succ_;
            ::tvlm::IL *il_;

            static int counter_; // 0;
            CfgNode(::tvlm::IL * ins) :
                    il_(ins), id_(counter_++) {

            }
            virtual ~CfgNode(){}

            CfgNode(const CfgNode &other) :
                    il_(other.il_), id_(other.id_), succ_(other.succ_), pred_(other.pred_) {}


            ::tvlm::IL *il() const {
                return il_;
            }

            virtual bool operator==(const CfgNode &other) const {
                return id_ == other.id_;
            }

            virtual bool operator<(const CfgNode &other) const {
                return id_ < other.id_;
            }
        };

        class CfgStmtNode : public CfgNode {
        public:
            CfgStmtNode(Instruction *ins)
                    : CfgNode(ins) {}
        };

        class CfgFunEntryNode : public CfgNode {
        public:
            CfgFunEntryNode(::tvlm::IL *fnc) :
                    CfgNode(fnc) {}
        };

        class CfgFunExitNode : public CfgNode {
        public:
            CfgFunExitNode(::tvlm::IL *fnc) :
                    CfgNode(fnc) {}
        };
    }
