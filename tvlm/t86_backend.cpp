#include "t86_backend.h"

#include <memory>
#include "t86/instruction.h"
#include "analysis/liveness_analysis.h"
#include "tvlm/tvlm/codeGeneration/InstructionSelection/NaiveIS.h"
#include "tvlm/codeGeneration/InstructionSelection/SuperNaiveIS.h"
#include "tvlm/codeGeneration/registerAllocation/SuperNaiveRegisterAllocator.h"
#include "tvlm/codeGeneration/registerAllocation/ColoringAllocator.h"
#include "tvlm/codeGeneration/Epilogue.h"

namespace tvlm{

    bool setupColorConfig(){
        tiny::config.setDefaultIfMissing("-colorRegisterAllocation", "1");
        int c;
        try{
        c= stoi(tiny::config.get("-colorRegisterAllocation"));
        }catch (...) {
            std::cerr << "read <" << "cannot" << std::endl;
            return true;
        }
        std::cerr << "read <" << c << std::endl;
            return c > 0;
    }


    t86_Backend::TargetProgram t86_Backend::compileToTarget(t86_Backend::IL &&il) {
        //auto codeGenerator = CodeGenerator (il);
//        auto callingConventionedIL = CallingConvention();
        auto selected =  SuperNaiveIS::translate(std::move(il));
        bool color = setupColorConfig();
        std::unique_ptr<RegisterAllocator> regAllocator;
        if(color){
            std::cerr << "using ColoringRA"<<std::endl;
            regAllocator = std::make_unique<ColoringAllocator>( std::move(selected));
        }else{
            std::cerr << "using NaiveRA" << std::endl;
            regAllocator = std::make_unique<SuperNaiveRegisterAllocator>( std::move(selected));

        }
        selected = std::move(regAllocator->run());
        auto index = 0;
        std::stringstream ss;
        auto printer = tiny::ASTPrettyPrinter(ss);
        bool again = regAllocator->changedProgram();
        while(again){
            ss.str("");
            selected.program_->print(printer);
            std::cerr << tiny::color::lightBlue << "IL" << index++  << ":\n" << ss.str() << std::endl;
            selected = std::move(SuperNaiveIS::translate(std::move(selected)));
            auto newregAllocator = ColoringAllocator(std::move(selected));
            selected = std::move(newregAllocator.run());
            again = newregAllocator.changedProgram();

        }
        ss.str("");
        selected.program_->print(printer);
        std::cerr << tiny::color::lightBlue << "IL" << index++  << ":\n" << ss.str() << std::endl;

        std::cerr << "-----------------------------------------------\n\n\n" << std::endl;
        ss.str("");
        selected.program_->printAlloc(printer);
        std::cerr << tiny::color::lightBlue << "Alloc Print" <<  ":\n" << ss.str() << std::endl;
        auto epiloged = NaiveEpilogue().translate(std::move(selected));
        return epiloged;
//            return tvlm::ILTiler::translate(il);
//        return NaiveIS::translate(il);
//        tiny::t86::ProgramBuilderOLD pb;
//        return std::move(pb.program());
    }

} //namespace tvlm

#include "tvlm/analysis/next_use_analysis.h"


#if (defined TARGET_t86)
int tvlm::t86_Backend::MemoryCellSize = 4;

void tvlm::t86_Backend::tests() {

}
// inBytes
#else
int tvlm::t86_Backend::MemoryCellSize = 1; // inBytes
#endif
