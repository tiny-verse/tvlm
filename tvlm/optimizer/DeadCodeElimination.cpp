#include "DeadCodeElimination.h"
#include "tvlm/analysis/constantPropagation_analysis.h"

namespace tvlm{

    void DeadCodeElimination::run(Pass::IL &il) {

//        for (int i = 0; i < 1000; i++) {
            for(auto fncIt = il.functions().begin();fncIt != il.functions().end();){
                auto * fnc =  fncIt->second.get();
                bool removed = false;
                if(this->unusedFunction(il, fncIt->first)){
                    fncIt = il.removeFunction(fnc);
                    removed = true; continue;
                }else{
                    fncIt++;
                }



                for( auto & bb : getpureFunctionBBs(fnc)){
                    optimizeBasicBlock(bb.get());
                }
            }
//        }
    }
    bool sideeffects(const Instruction * ins){
        return  dynamic_cast<const CallStatic*>(ins) ||
                dynamic_cast<const Call*>(ins) ||
                dynamic_cast<const Store*>(ins) ||
                dynamic_cast<const PutChar*>(ins) ||
                dynamic_cast<const GetChar*>(ins) ||
                dynamic_cast<const Return*>(ins) ||
                dynamic_cast<const Jump*>(ins) ||
                dynamic_cast<const CondJump*>(ins) ||
                dynamic_cast<const StructAssign*>(ins) ||
                dynamic_cast<const Halt*>(ins) ;

    }

    void DeadCodeElimination::optimizeDeadCodeElimination(BasicBlock *bb) {
        std::vector<std::unique_ptr<ILInstruction>> newBBInstructions;
        for(auto & instr : getpureBBsInstructions(bb)) {
            //instruction is dead while it is nowhere used
            // since this information is in each instruction it doesnt require analysis

            //carefull! some instructions are not used anywhere but has side effects

            //now we want to get rid of every instruction that doesnt need to be in the BB so no replacing with NOP (the harder way)

            //NO NOop
            //Copy sideeffecting or instructions with usages (NO with empty usages)
            if(! dynamic_cast<NOPInstruction*>(instr.get()) && ( sideeffects(instr.get()) || !instr->usages().empty())){
                newBBInstructions.emplace_back(std::move(instr));
            }
        }
        getpureBBsInstructions(bb) = std::move(newBBInstructions);
    }

    void DeadCodeElimination::optimizeBasicBlock(BasicBlock *bb) {
        optimizeDeadCodeElimination(bb);
    }

    bool DeadCodeElimination::unusedFunction(Pass::IL &program, const Symbol &symbol) {
        if(symbol.name() == "main"){
            return false; // cannot remove main fnc
        }


        std::map<Symbol, std::set<Instruction::CallInstruction*>> callFrequency;
        bool programContainsIndirectCall = false;

        for ( auto * gIns : getBBsInstructions(getProgramsGlobals(&program))){
            if(dynamic_cast<Call*>(gIns)){
                return false; // indirect call present
            }else if (auto call = dynamic_cast<CallStatic*>(gIns)){
                callFrequency[ call->f()->name()].emplace( call);
            }
        }
        for (auto fnc : getProgramsFunctions(&program)) {
            for(auto bb : getFunctionBBs(fnc.second)){
                for(auto ins : getBBsInstructions(bb)){
                    if(dynamic_cast<Call*>(ins)){
                        return false; // indirect call present
                    }else if (auto call = dynamic_cast<CallStatic*>(ins)){
                        callFrequency[ call->f()->name()].emplace( call);
                    }
                }
            }
        }

        if(callFrequency[symbol].empty()){
            return true;
        }
        return false;
    }
}