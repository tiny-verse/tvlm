#include "ConstantPropagation.h"
#include "tvlm/il/il.h"
#include "tvlm/tvlm/analysis/constantPropagation_analysis.h"
#include <cmath>

namespace tvlm{


    int64_t ConstantPropagation::resolveUnOperator(UnOp * un, int64_t op) {

        if (un->opType() == UnOpType::DEC) {
            return --op;
        } else if (un->opType() == UnOpType::INC) {
            return ++op;
        } else if (un->opType() == UnOpType::NOT) {
            return ~op;
        } else if (un->opType() == UnOpType::UNSUB) {
            return -op;
        } else {
            throw "[ConstantPropagation] unknown UnOpType (integer)";
        }
    }


    double ConstantPropagation::resolveUnOperator(UnOp * un, double op) {

        if (un->opType() == UnOpType::DEC) {
            return --op;
        } else if (un->opType() == UnOpType::INC) {
            return ++op;

        }else  if (un->opType() == UnOpType::UNSUB) {
            return -op;

        } else {
            throw "[ConstantPropagation] unknown UnOpType (integer)";
        }

    }

    int64_t ConstantPropagation::resolveBinOperator(BinOp * bin, int64_t lhs, int64_t rhs){

        auto binIns = dynamic_cast<Instruction::BinaryOperator *>(bin);
        if(binIns){
            if(binIns->opType() == BinOpType ::ADD){
                return lhs + rhs;
            }else if(binIns->opType() == BinOpType::SUB){
                return lhs - rhs;

            }else if(binIns->opType() == BinOpType::MUL){
                return lhs * rhs;

            }else if(binIns->opType() == BinOpType::DIV){
                return lhs / rhs;

            }else if(binIns->opType() == BinOpType::LSH){
                return lhs<< rhs;

            }else if(binIns->opType() == BinOpType::RSH){
                return lhs >> rhs;

            }else if(binIns->opType() == BinOpType::MOD){
                return lhs % rhs;

            }else if(binIns->opType() == BinOpType::AND){
                return lhs & rhs;

            }else if(binIns->opType() == BinOpType::OR){
                return lhs | rhs;

            }else if(binIns->opType() == BinOpType::XOR){
                return lhs ^ rhs;

            }else if(binIns->opType() == BinOpType::LT){
                return lhs < rhs;

            }else if(binIns->opType() == BinOpType::LTE){
                return lhs <= rhs;

            }else if(binIns->opType() == BinOpType::GT){
                return lhs > rhs;

            }else if(binIns->opType() == BinOpType::GTE){
                return lhs >= rhs;

            }else if(binIns->opType() == BinOpType::EQ){
                return lhs == rhs;

            }else if(binIns->opType() == BinOpType::NEQ){
                return lhs != rhs;

            }else {
                throw "[ConstantPropagation] unknown BinOpType";
            }
        }else{
            throw "[ConstantPropagation]Cannot bin optimize non-BionOp instruciton";

        }
    }
    double ConstantPropagation::resolveBinOperator(BinOp * bin, double lhs, double rhs){

        auto binIns = dynamic_cast<Instruction::BinaryOperator *>(bin);
        if(binIns){
            if(binIns->opType() == BinOpType ::ADD){
                return lhs + rhs;
            }else if(binIns->opType() == BinOpType::SUB){
                return lhs - rhs;

            }else if(binIns->opType() == BinOpType::MUL){
                return lhs * rhs;

            }else if(binIns->opType() == BinOpType::DIV){
                return lhs / rhs;

            }else if(binIns->opType() == BinOpType::LT){
                return lhs < rhs;

            }else if(binIns->opType() == BinOpType::LTE){
                return lhs <= rhs;

            }else if(binIns->opType() == BinOpType::GT){
                return lhs > rhs;

            }else if(binIns->opType() == BinOpType::GTE){
                return lhs >= rhs;

            }else if(binIns->opType() == BinOpType::EQ){
                return lhs == rhs;

            }else if(binIns->opType() == BinOpType::NEQ){
                return lhs != rhs;

            }else {
                throw "[ConstantPropagation] unknown BinOpType";
            }
        }else{
            throw "[ConstantPropagation]Cannot bin optimize non-BionOp instruciton";

        }
    }

    bool ConstantPropagation::isPowerOfTwo(int64_t num){
        return (num & (num - 1)) == 0; // value == 2^(whatever)
    }

    void ConstantPropagation::optimizeStrengthReduction(BasicBlock * bb) {
        auto insns = getBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size(); idx < len; idx++) {
            auto &first = insns[idx];
            auto binOp = dynamic_cast<BinOp *>(first);
            if (binOp && binOp->resultType() == ResultType::Integer) {
                if(binOp->opType() == BinOpType::MUL){
                    if (auto lhs = dynamic_cast<Instruction::ImmValue *>(binOp->lhs())) {
                        if (isPowerOfTwo(lhs->valueInt())) {
                            auto instrName = first->name();
                            auto lhsName = lhs->name();
                            int64_t newValue = (int64_t)log2(lhs->valueInt());

                            auto newInstr = bb->replaceInstr(lhs, new LoadImm(newValue, lhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::LSH, Instruction::Opcode::LSH, binOp->rhs(), newInstr, first->ast()));
                            first->setName(instrName);
                            lhs->setName(lhsName);

                        }
                    }else if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->lhs())){
                        if (isPowerOfTwo(rhs->valueInt())) {
                            auto instrName = first->name();
                            auto lhsName = rhs->name();
                            int64_t newValue = (int64_t)log2(rhs->valueInt());

                            auto newInstr = bb->replaceInstr(lhs, new LoadImm(newValue, rhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::LSH, Instruction::Opcode::LSH, binOp->lhs(), newInstr, first->ast()));
                            first->setName(instrName);
                            rhs->setName(lhsName);

                        }
                    }
                }
                else if(binOp->opType() == BinOpType::DIV){

                    if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->rhs())) {
                        if (isPowerOfTwo(rhs->valueInt())) {
                            auto instrName = first->name();
                            auto rhsName = rhs->name();
                            int64_t newValue = (int64_t)log2(rhs->valueInt());
                            auto newInstr = bb->replaceInstr(rhs, new LoadImm(newValue, rhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::RSH,Instruction::Opcode::RSH, binOp->lhs(), newInstr, first->ast()));

                            first->setName(instrName);
                            rhs->setName(rhsName);

                        }
                    }
                }
                else if(binOp->opType() == BinOpType::MOD){

                    if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->rhs())) {
                        if (isPowerOfTwo(rhs->valueInt())) {
                            auto instrName = first->name();
                            auto rhsName = rhs->name();
                            int64_t newValue = (int64_t)log2(rhs->valueInt());
                            auto newInstr = bb->replaceInstr(rhs, new LoadImm(newValue, rhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::AND,Instruction::Opcode::AND, binOp->lhs(), rhs, first->ast()));

                            first->setName(instrName);
                            rhs->setName(rhsName);

                        }
                    }
                }

            }else if(binOp && binOp->resultType() == ResultType::Double){

            }

        }
    }

    void ConstantPropagation::optimizeBasicBlock(BasicBlock* bb, CPNodeState & analysis ) {
        optimizeConstantPropagation(bb, analysis);
        optimizeStrengthReduction(bb);
    }


    void ConstantPropagation::run(IL & il){
        auto constProp = new ConstantPropagationAnalysis<> (&il);
        auto an  = constProp->analyze();
        CPNodeState analysis = constProp->transform(an);

        for(auto & fnc : il.functions()){
            auto & bbs = getpureFunctionBBs(fnc.second.get());
            for( auto & bb : bbs){
                optimizeBasicBlock(bb.get(), analysis);
            }
            for( auto & bb : bbs){
                optimizeConstantPropagationLatePass(bb.get(), analysis);
            }
            for( auto & bb : bbs){
                optimizeConstantPropagationLateLatePass(bb.get(), analysis);
            }
        }
    }

    void ConstantPropagation::optimizeConstantPropagationLatePass(BasicBlock * bb,  CPNodeState & analysis) {
        auto & insns = getpureBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size();  idx < len; idx++) {
            auto &first = insns[idx];
            if (auto store = dynamic_cast<Store *>(first.get())){
                auto it = analysis.find((ILInstruction*)store->address());
                if(it == analysis.end()){
//                    std::cerr << "cannot find in analysis, dead code?";
                }else{
                    auto tmp = store->address()->usages();
                    for (auto it = tmp.begin(); it != tmp.end();) {
                        auto anotherStore = dynamic_cast<Store*>(*it);
                        if( (dynamic_cast<AllocG*>(*it) || dynamic_cast<AllocL*>(*it)) &&
                            (*it == store || (anotherStore && anotherStore->address() == store->address() /*do not remove while taken pointer from*/) )){
                            //
                            it = tmp.erase(it);
                        }else{it++;}
                    }


                    if ( tmp.empty() && dynamic_cast<FlatVal<Constant>*>(it->second)) {//found in analysis as constant
                        //remove Store while obsolete
                        bb->removeInstr(store);
                    }
                }
            }

        }
    }
    void ConstantPropagation::optimizeConstantPropagationLateLatePass(BasicBlock * bb,  CPNodeState & analysis) {
        auto & insns = getpureBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size();  idx < len; idx++) {
            auto &first = insns[idx];
            if (auto allocL = dynamic_cast<AllocL *>(first.get())){
                auto it = analysis.find((ILInstruction*)allocL);
                if(it == analysis.end()){
//                    std::cerr << "cannot find in analysis, dead code?";
                }else{

                    if (allocL->usages().empty() &&  dynamic_cast<FlatVal<Constant>*>(it->second)) {//found in analysis as constant
                        //remove Store while obsolete
                        bb->removeInstr(allocL);
                    }
                }
            }
            else if (auto allocG = dynamic_cast<AllocG *>(first.get())){
                auto it = analysis.find((ILInstruction*)allocG);
                if(it == analysis.end()){
//                    std::cerr << "cannot find in analysis, dead code?";
                }else{
                    if (allocG->usages().empty() &&  dynamic_cast<FlatVal<Constant>*>(it->second)) {//found in analysis as constant
                        //remove Store while obsolete
                        bb->removeInstr(allocG);
                    }
                }
            }

        }
    }
    void ConstantPropagation::optimizeConstantPropagation(BasicBlock * bb,  CPNodeState & analysis) {
        auto & insns = getpureBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size();  idx < len; idx++) {
            auto & first = insns[idx];
            auto binOp = dynamic_cast<BinOp*>(first.get());
            if (binOp ){
                    if(binOp->lhs()->resultType() == ResultType::Double){
                        if(binOp->rhs()->resultType() == ResultType::Double){
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    double value = resolveBinOperator(binOp,lhs->valueFloat() , rhs->valueFloat()) ;
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }else{
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    double value = resolveBinOperator(binOp,lhs->valueFloat() , (double)rhs->valueInt());
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }
                    }else {
                        if(binOp->rhs()->resultType() == ResultType::Double){
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    double value = resolveBinOperator(binOp,(double)lhs->valueInt() , rhs->valueFloat());
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }else{
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    int64_t value = resolveBinOperator(binOp,lhs->valueInt() , rhs->valueInt()) ;
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }
                    }
            }
            else if(auto un = dynamic_cast<UnOp*>(first.get())){
                if(auto op = dynamic_cast<LoadImm*>(un->operand())){
                    if(op->resultType() == ResultType::Double){
                        double value = resolveUnOperator(un,op->valueFloat() ) ;
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, un->ast()));
                        first->setName(regName);
                        bb->removeInstr(op);
                    }else{

                        int64_t value = resolveUnOperator(un,op->valueInt() ) ;
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, un->ast()));
                        first->setName(regName);
                        bb->removeInstr(op);
                    }
                }
            }
            else if(auto trunc = dynamic_cast<Truncate *>(first.get())){

                if(auto src = dynamic_cast<LoadImm*>(trunc->src())) {
                    if (src->resultType() == ResultType::Double) {

                        int64_t value = (int64_t)src->valueFloat();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, trunc->ast()));
                        first->setName(regName);

                        bb->removeInstr(src);
                    } else {

                        int64_t value = src->valueInt();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, trunc->ast()));
                        first->setName(regName);
                        bb->removeInstr(src);
                    }
                }
            }
            else if(auto extend = dynamic_cast<Extend *>(first.get())){

                if(auto src = dynamic_cast<LoadImm*>(extend->src())) {
                    if (src->resultType() == ResultType::Double) {

                        double value = src->valueFloat();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, extend->ast()));
                        first->setName(regName);

                        bb->removeInstr(src);
                    } else {

                        double value = (double)src->valueInt();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, extend->ast()));
                        first->setName(regName);
                        bb->removeInstr(src);
                    }
                }
            }
            else if (auto load = dynamic_cast<Load *>(first.get())){
                auto it = analysis.find((ILInstruction*)load->address());
                if(it == analysis.end()){
                    std::cerr << "cannot find in analysis, dead code?";
                }else{
                    if( dynamic_cast<FlatVal<Constant>*>(it->second)){

                        //found in analysis as constant
                        auto constant = it->second->get();
                        //replace load with load of a constant

                        auto regName = first->name();
                        if(constant.resType() == ResultType::Double){

                            bb->replaceInstr(first.get(), new LoadImm(constant.getFloat(), load->ast()));
                        }else{
                            bb->replaceInstr(first.get(), new LoadImm(constant.getInt(), load->ast()));
                        }
                        first->setName(regName);
                        bb->removeInstr(load);

                    }
                }

            }
        }

    }

}