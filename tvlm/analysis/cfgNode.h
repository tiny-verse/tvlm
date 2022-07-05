#pragma once
#include <set>
#include "tvlm/tvlm/il/il.h"

using Instruction = ::tvlm::Instruction;
using Function  = ::tvlm::Function;


namespace tvlm {

    class TilingRule;

    template<class T>
    class CfgNode {
    public:
        int id_;
        std::vector<CfgNode<T>*> pred_;
        std::vector<CfgNode<T>*> succ_;
        IL *il_;
//            std::vector<TilingRule * > possibleRules_;
//            TilingRule * selectedRule_;
        T * innerInfo;

        static int counter_; // 0;

        explicit CfgNode(IL * ins) ;
        virtual ~CfgNode() = default;

        CfgNode(const CfgNode<T> &other);

        IL *il() const {
            return il_;
        }

        virtual bool operator==(const CfgNode<T> &other) const {
            return id_ == other.id_;
        }

        virtual bool operator<(const CfgNode<T> &other) const {
            return id_ < other.id_;
        }
    };


    template<class T>
    class CfgStmtNode : public CfgNode<T> {
    public:
        CfgStmtNode(Instruction *ins);
    };


    template<class T>
    class CfgFunEntryNode : public CfgNode<T> {
    public:
        CfgFunEntryNode(IL *fnc);

    };


    template<class T>
    class CfgFunExitNode : public CfgNode<T> {
    public:
        CfgFunExitNode(IL *fnc) ;
    };
//******************************************************************************************


    template<typename T>
    int CfgNode<T>::counter_ = 0;


    template<class T>
    CfgNode<T>::CfgNode(IL *ins):
            il_(ins), id_(counter_++) {

    }

    template<class T>
    CfgNode<T>::CfgNode(const CfgNode<T> &other) :
            il_(other.il_), id_(other.id_), succ_(other.succ_), pred_(other.pred_) {}



    template<class T>
    CfgStmtNode<T>::CfgStmtNode(Instruction *ins):
            CfgNode<T>(ins) {}

    template<class T>
    CfgFunEntryNode<T>::CfgFunEntryNode(IL *fnc) :
            CfgNode<T>(fnc) {}


    template<class T>
    CfgFunExitNode<T>::CfgFunExitNode(IL *fnc):
            CfgNode<T>(fnc) {}

}
