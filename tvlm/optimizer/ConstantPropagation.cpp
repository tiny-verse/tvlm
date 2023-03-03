#include "ConstantPropagation.h"
#include "tvlm/il/il.h"

#include <cmath>



namespace tvlm{


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

    bool ConstantPropagation::isPowerOfTwo(int num){
        return (num & (num - 1)) == 0; // value == 2^(whatever)
    }

    void ConstantPropagation::optimizeStrengthReduction(BasicBlock * bb) {
        auto insns = getBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size(); idx < len; idx++) {
            auto &first = insns[idx];
            auto binOp = dynamic_cast<BinOp *>(first);
            if (binOp && binOp->resultType() == ResultType::Integer) {
                if (auto lhs = dynamic_cast<Instruction::ImmValue *>(binOp->lhs())) {
                    if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->rhs())) {
                        int64_t value = resolveBinOperator(binOp, lhs->valueInt(), rhs->valueInt());

                        bb->replaceInstr(first, new LoadImm(value, first->ast()));
                        bb->removeInstr(lhs);
                        bb->removeInstr(rhs);
//                        first.replaceMe(value)
//                        new(first) Instruction::ImmValue(value);
//                        new(lhs) Instruction::NOP();
//                        new(rhs) Instruction::NOP();
                    }
                }
                if(binOp->opType() == BinOpType::MUL){
                    if (auto lhs = dynamic_cast<Instruction::ImmValue *>(binOp->lhs())) {
                        if (isPowerOfTwo(lhs->valueInt())) {
                            auto instrName = first->name();
                            auto lhsName = lhs->name();
                            int64_t newValue = log2(lhs->valueInt());

                            bb->replaceInstr(lhs, new LoadImm(newValue, lhs->ast()));
//                        new(lhs) Instruction::ImmValue(newValue);
                            bb->replaceInstr(first, new BinOp( BinOpType::LSH, Instruction::Opcode::LSH, binOp->rhs(), lhs, first->ast()));
//                        new(first) Instruction::Shl(i->rhs(), lhs);

                            first->setName(instrName);
                            lhs->setName(lhsName);

                        }
                    }

                }
                if(binOp->opType() == BinOpType::DIV){

                    if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->rhs())) {
                        if (isPowerOfTwo(rhs->valueInt())) {
                            auto instrName = first->name();
                            auto rhsName = rhs->name();
                            int64_t newValue = log2(rhs->valueInt());
//                            new(rhs) Instruction::ImmValue(newValue);
                            bb->replaceInstr(rhs, new LoadImm(newValue, rhs->ast()));
//                            new(first) Instruction::Shr(i->lhs(), rhs);
                            bb->replaceInstr(first, new BinOp( BinOpType::RSH,Instruction::Opcode::RSH, binOp->lhs(), rhs, first->ast()));

                            first->setName(instrName);
                            rhs->setName(rhsName);

                        }
                    }


                }



            }else if(binOp && binOp->resultType() == ResultType::Double){
                if (auto lhs = dynamic_cast<Instruction::ImmValue *>(binOp->lhs())) {
                    if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->rhs())) {
                        double value ;
                        if(lhs->resultType() == ResultType::Integer
                            && rhs->resultType() == ResultType::Double){
                            value= resolveBinOperator(binOp, (double)lhs->valueInt(), rhs->valueFloat());
                        }else if (lhs->resultType() == ResultType::Double
                            && rhs->resultType() == ResultType::Integer
                        ){
                            value= resolveBinOperator(binOp, lhs->valueFloat(), (double)rhs->valueInt());
                        }else {
                            value= resolveBinOperator(binOp, lhs->valueFloat(), rhs->valueFloat());
                        }

                        bb->replaceInstr(first, new LoadImm(value, first->ast()));
                        bb->removeInstr(lhs);
                        bb->removeInstr(rhs);
//                        first.replaceMe(value)
//                        new(first) Instruction::ImmValue(value);
//                        new(lhs) Instruction::NOP();
//                        new(rhs) Instruction::NOP();
                    }
                }
            }

        }
    }


    void ConstantPropagation::optimizeBasicBlock(BasicBlock* bb) {
        optimizeConstantPropagation(bb);
        optimizeStrengthReduction(bb);
    }


    void ConstantPropagation::run(IL & il){
        for (int i = 0; i < 1000; i++) {
            for(auto & fnc : il.functions()){
                for( auto & bb : getpureFunctionBBs(fnc.second.get())){
                    optimizeBasicBlock(bb.get());
                }
            }
        }
    }

    void ConstantPropagation::optimizeConstantPropagation(BasicBlock * bb) {
        auto & insns = getpureBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size();  idx < len; idx++) {
            auto & first = insns[idx];
            auto binOp = dynamic_cast<BinOp*>(first.get());
            if (binOp && binOp->resultType() == ResultType::Integer){
                if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                    if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                        int64_t value = resolveBinOperator(binOp,lhs->valueInt() , rhs->valueInt()) ;

//                        auto regNum = first->regNum_;
                        auto regName = first->name();


//                        new ( first.get()) Instruction::LoadImm(value);
                        bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
//                        new ( first.get()) Instruction::LoadImm(value);

//                        first->setRegNumber(regNum);
                        first->setName(regName);

//                        new (lhs) NOPInstruction();
                        bb->removeInstr(lhs);
                        bb->removeInstr(rhs);
                    }
                }
            } else if(auto un = dynamic_cast<Instruction::UnaryOperator*>(first.get())){
//                int64_t value = resolveUnaryOperator()
            }
        }

    }



}