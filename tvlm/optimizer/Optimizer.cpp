#include "Optimizer.h"

#include "FunctionInliner.h"
#include "ConstantPropagation.h"

namespace tvlm{
    void Optimizer::optimize(IR &ir) {

        std::stringstream ss;
        auto printer = tiny::ASTPrettyPrinter(ss);
        ir.print(printer);
        std::cerr << tiny::color::lightBlue << "IL before optimizer" << ":\n" << ss.str() << std::endl;


        for(auto & pass : passes_ ){
            pass->run(ir);
        }
        ss.str("");
        ir.print(printer);
        std::cerr << tiny::color::lightBlue << "IL after optimizer" << ":\n" << ss.str() << std::endl << "||" << std::endl;
//
//        functionInlining(ir);
//        for (int i = 0; i < 1000; i++) {
//            for(auto & fnc : ir.functions()){
//                for( auto & bb : getFunctionBBs(fnc.second.get())){
//                    optimizeBasicBlock(bb.get());
//                }
//            }
//        }
    }


    void LastPass::run(IL & il){
        for(auto & fnc : il.functions()){ // for every function
            auto & bbs = getpureFunctionBBs(fnc.second.get());
            for(auto bb = bbs.begin(); bb != bbs.end() ;){
                //remove empty thus not terminated BBs
                if(!(*bb)->terminated() ){
                    bb = bbs. erase(bb);
                    continue;
                }

                auto & insns = getpureBBsInstructions(bb->get());
                for(auto ins =  insns.begin(); ins != insns.end(); ){//for every ins <- bb <- fnc
                    //remove NOP instructions
                    if(dynamic_cast<NOPInstruction*>((ins->get()))){
                        ins = insns.erase(ins);
                        continue;
                    }
                    ins++;
                }
                bb++;
            }
        }
    }

    void Optimizer::initialize_passes() {
        passes_.push_back(std::make_unique<FunctionInliner>());
        passes_.push_back(std::make_unique<ConstantPropagation>());
        passes_.push_back(std::make_unique<LastPass>());
    }

}