#pragma once

#include <stack>
#include "RegisterAllocator.h"
#include "t86/instruction.h"
#include "t86/program.h"
#include "t86/program/programbuilder.h"
#include "tvlm/codeGeneration/ProgramBuilder.h"

#include "tvlm/analysis/liveness_analysisTartget.h"
#include "SuperNaiveRegisterAllocator.h"


/**
 * Preparation:
 *      Instruction Selection done - TInstruction known
 *          - Nontrivial
 *      Naively assigned Registers
 *      Liveness analysis
 *          - Cfg construction
 *      ColorPicking
 *          -make graph:
 *              -verticies: live range
 *              -edges: overlapping ranges
 *          - each live range gets color (Register) by rules:
 *              1) K colors: node with degree k-1 safely colored
 *              2) coloring this node remove them from graph (might enable other nodes)
 *              3) no such colorable? -> "(spill)" : might enable condition 1)
 *              removing from graph == push to regStack -> determines color
 *
 * Work:
 *      Rework assigned registers by idea of graph coloring
 * */
namespace tvlm{

    class ColoringAllocator : public SuperNaiveRegisterAllocator{
    public:
        ColoringAllocator(TargetProgram && prog):
        SuperNaiveRegisterAllocator(std::move(prog)),
        la_(new ColoringLiveAnalysis<>(&targetProgram_)),
        programChanged_(false){
        }
        ColoringAllocator(ColoringAllocator && alloc) = default;

        bool changedProgram()const override{
            return programChanged_;
        }
        TargetProgram  run(TargetProgram &&  prog){
            targetProgram_ = std::move(prog);
            return std::move(this->run());
        }

        TargetProgram run()override{

        //----Preparation----
            //LivenessAnalysis
            std::shared_ptr<Program>  prog = getProgram(&targetProgram_);
            programChanged_ = false;

                analysisResult_ = la_->analyze();
                auto tmp = la_->getLiveRanges();
                for (auto * i : tmp) {
                    addLR(std::move(std::unique_ptr<CLiveRange>(i)));
                }

                getLRBundles();

                //ColorPicking
                !generateLiveRanges();

            //Next convert result of colorPicking to update VirtualRegisterPlaceholders

            //implement logic of passing through the program;
            if(programChanged_){
                return std::move(targetProgram_);
            }

            setColors();



            RegisterAllocator::visit(getProgram(&targetProgram_).get());;
            return std::move(targetProgram_);

        }

        virtual ~ColoringAllocator(){

        }

    protected:

        std::unique_ptr<ColoringLiveAnalysis<>> la_;
        MAP<const CfgNode<> *,std::set<CLiveRange*>> analysisResult_;


        void addLR(std::unique_ptr<CLiveRange> && lr);
        //incidence graph
        std::map<const IL *, size_t> searchRanges_; // size_t -> index to liveRanges
        std::map<CLiveRange *, size_t> lrIndex_; //  liveRange to index
        std::map<size_t, std::set<Instruction *>> searchInstrs_; // size_t -> index to liveRanges
        std::vector<std::unique_ptr<CLiveRange>> liveRanges_;
        std::vector<std::set<size_t>> LRincidence_;
        std::set<Instruction *> unusedInstructions_;

        std::map<const Instruction *, size_t> spillIndexes_; //size_t -> index in liveRanges_ //both: 1st where to spill ; 2nd: what to spill
        std::stack<size_t> colorPickingStack_; //size_t -> index in liveRanges_
        std::map<const Instruction * , size_t> colorPickingResult_;
        std::map<size_t , size_t> colorPickingSemiResult_;

        std::map<size_t, VirtualRegister>finalMapping_;
        std::map<size_t, VirtualRegister>finalFMapping_;

        bool programChanged_;
        //create live ranges, and create incidence graph
        bool generateLiveRanges(); // true == assigned without spilling
        void getLRBundles(); //
        bool setColors();

        VirtualRegister getRegToSpill() override;
        VirtualRegister getReg( Instruction *ins) override;
        VirtualRegister getFReg( Instruction *ins) override;
        void callingConvCallerSave(const Instruction *currIns) override;
        void resetFreeRegs(const Instruction * except = nullptr) override;

    };
}
