#include "Optimizer.h"

#include "FunctionInliner.h"
#include "ConstantPropagation.h"
#include "DeadCodeElimination.h"

namespace tvlm{
    void Optimizer::optimize(IL &ir) {

         std::stringstream ss;
        auto printer = tiny::ASTPrettyPrinter(ss);
        ir.print(printer);
        std::cerr << tiny::color::lightBlue << "IL before optimizer" << ":\n" << ss.str() << std::endl;


        for(auto & pass : passes_ ){
             pass->run(ir);

            ss.str("");
            ir.print(printer);
            std::cerr << tiny::color::lightBlue << "IL after "<< pass->name() << ":\n" << ss.str() << std::endl << "||" << std::endl;
        }
        ss.str("");
        ir.print(printer);
        std::cerr << tiny::color::lightBlue << "IL after optimizer" << ":\n" << ss.str() << std::endl << "||" << std::endl;

    }


    void LastPass::run(IL & il){
        for(auto & fnc : il.functions()){ // for every function
            auto & bbs = getpureFunctionBBs(fnc.second.get());
            for(auto bb = bbs.begin(); bb != bbs.end() ;){
                //remove empty thus not terminated BBs
                if(!(*bb)->terminated() || (bb->get()->used().empty() && bb != bbs.begin())){
                    // first bb of a function needs to stay allways
                    fnc.second->removeBB(bb->get());
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
        if(inlining){
            passes_.push_back(std::make_unique<FunctionInliner>());
//            if(deadCodeElimination){
//                passes_.push_back(std::make_unique<DeadCodeElimination>());
//            }
        }
        if(constant){
            passes_.push_back(std::make_unique<ConstantPropagation>());
        }
        if(deadCodeElimination){
            passes_.push_back(std::make_unique<DeadCodeElimination>());
        }
        passes_.push_back(std::make_unique<LastPass>());
    }

}