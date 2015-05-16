#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Type.h"
#include <vector>
#include <iostream>
#include <algorithm>
 
using namespace llvm;
using namespace std;
 
namespace {


  // https://github.com/thomaslee/llvm-demo/blob/master/main.cc
  static Function* printf_prototype(LLVMContext& ctx, Module *mod) {
    std::vector<Type*> printf_arg_types;
    printf_arg_types.push_back(Type::getInt8PtrTy(ctx));
 
    FunctionType* printf_type = FunctionType::get(Type::getInt32Ty(ctx), printf_arg_types, true);
    Function *func = mod->getFunction("printf");
    if(!func)
      func = Function::Create(printf_type, Function::ExternalLinkage, Twine("printf"), mod);
    func->setCallingConv(CallingConv::C);
    return func;
  }
  
  struct CS201PathProfiling : public FunctionPass {
    static char ID;
    LLVMContext *Context;
    CS201PathProfiling() : FunctionPass(ID) {}
    GlobalVariable *bbCounter = NULL;
    GlobalVariable *BasicBlockPrintfFormatStr = NULL;
    Function *printf_func = NULL;
//--------------->>>>>>>>>>>>>> 
/*
	struct pairs{
		BasicBlock * block;
		std::vector<BasicBlock*> domBlock;
	};
*/	
	std::vector<BasicBlock*> BBs;// = new std::vector<BasicBlock*> ;
	std::vector<BasicBlock*> dominators; //new std::vector<BasicBlock*>;

	





//-----------------<<<<<<<<<<<<<<

    //----------------------------------
    bool doInitialization(Module &M) {
      errs() << "\n---------Starting BasicBlockDemo---------\n";
      Context = &M.getContext();
      bbCounter = new GlobalVariable(M, Type::getInt32Ty(*Context), false, GlobalValue::InternalLinkage, ConstantInt::get(Type::getInt32Ty(*Context), 0), "bbCounter");
      const char *finalPrintString = "BB Count: %d\n";
      Constant *format_const = ConstantDataArray::getString(*Context, finalPrintString);
      BasicBlockPrintfFormatStr = new GlobalVariable(M, llvm::ArrayType::get(llvm::IntegerType::get(*Context, 8), strlen(finalPrintString)+1), true, llvm::GlobalValue::PrivateLinkage, format_const, "BasicBlockPrintfFormatStr");
      printf_func = printf_prototype(*Context, &M);
 
      errs() << "Module: " << M.getName() << "\n";

	
 
      return true;
    }
 
    //----------------------------------
    bool doFinalization(Module &M) {
      errs() << "-------Finished CS201PathProfiling----------\n";
      errs() << "basic blocks: \n";




//add...      
      for(auto *B: BBs)
          errs() << B->getName() <<", ";





      errs()<<"\n";

      return false;
    }
    
    //----------------------------------
    bool runOnFunction(Function &F) override {
      errs() << "Function: " << F.getName() << '\n';
 	int count = 0;
      for(auto &BB: F) {
        // Add the footer to Main's BB containing the return 0; statement BEFORE calling runOnBasicBlock
        if(F.getName().equals("main") && isa<ReturnInst>(BB.getTerminator())) { // major hack?
          addFinalPrintf(BB, Context, bbCounter, BasicBlockPrintfFormatStr, printf_func);
        }


//add..................................................
        BasicBlock *ptr_BB = &BB; 
	ptr_BB->setName(std::to_string(count));
        BBs.push_back(ptr_BB);
	
	count++;



//..........................................................
        runOnBasicBlock(BB);
      }
	formDominator(F);	
	
 
      return true; // since runOnBasicBlock has modified the program
    }
 
    //----------------------------------
    bool runOnBasicBlock(BasicBlock &BB) {
      errs() << "BasicBlock: " << BB.getName() << '\n';
      IRBuilder<> IRB(BB.getFirstInsertionPt()); // Will insert the generated instructions BEFORE the first BB instruction
 
      Value *loadAddr = IRB.CreateLoad(bbCounter);
      Value *addAddr = IRB.CreateAdd(ConstantInt::get(Type::getInt32Ty(*Context), 1), loadAddr);
      IRB.CreateStore(addAddr, bbCounter);
 
      for(auto &I: BB)
        errs() << I <<"<---this is "<< "\n";
 
      return true;
    }

//////////////////////////////////////////////
	void formDominator(Function &F)
	{

		// initialize, 
		unsigned size = BBs.size();
		unsigned i = 1;
		unsigned j = 0;
//		std::pair <BasicBlock*, std::vector<BasicBlock*> > pairs[size]; 
		vector<pair<BasicBlock*, std::vector<BasicBlock*> > > dompairs;
		vector<BasicBlock*> BBsTemp = BBs;
		BBsTemp.erase(BBsTemp.begin());
		vector<BasicBlock*> BBsInit;
		BBsInit.push_back(BBs[0]); 
		dompairs.push_back(make_pair(BBs[0],BBsInit));
//begin print
		errs() << dompairs[0].first->getName() << ": ";
		j = 0;
		while(j < dompairs[0].second.size()){
			errs() << ((dompairs[0].second)[j])->getName() <<", ";
			j++;
		}
		errs() << "\n";
//end print
		while(i < size)
		{
			vector<BasicBlock*> BBsT = BBs;
			
			dompairs.push_back(std::make_pair(BBs[i],BBsT));
//begin print		
			errs() << dompairs[i].first->getName() << ": ";
			j = 0;
			while(j < dompairs[i].second.size()){
				errs() << ((dompairs[i].second)[j])->getName() <<", ";
				j++;
			}
			errs() << "\n";
//end print			
			i++;
		}
		// end initialization

		bool change = true;
		unsigned cnt;
		while(change)
		{
			change = false;
			for(cnt = 0; cnt < dompairs.size();cnt++)
			{
				vector<BasicBlock*> newdoms;
				newdoms.push_back(dompairs[cnt].first);
				BasicBlock * Bb = dompairs[cnt].first;
				vector<BasicBlock*> temp;
				temp = BBs;
				vector<BasicBlock*>::iterator it;

				for(pred_iterator PI = pred_begin(Bb),E=pred_end(Bb); PI != E; ++PI)
				{
					BasicBlock *Pred = *PI;
					//intersection
					string name = Pred->getName();
					int domNum = atoi(name.c_str());
					unsigned tempSize = temp.size();
					unsigned pairSize = dompairs[domNum].second.size();
					it = std::set_intersection(temp.begin(), temp.end(),(dompairs[domNum].second).begin(), (dompairs[domNum].second).end(), temp.begin());
					
				}
//begin print
				errs() << "it's temp:   \n";
				for(auto *B : temp)
					errs() << B->getName() << ", ";				
				
			}
		}
		//loop
		
	

	}
 
    //----------------------------------
    // Rest of this code is needed to: printf("%d\n", bbCounter); to the end of main, just BEFORE the return statement
    // For this, prepare the SCCGraph, and append to last BB?
    void addFinalPrintf(BasicBlock& BB, LLVMContext *Context, GlobalVariable *bbCounter, GlobalVariable *var, Function *printf_func) {
      IRBuilder<> builder(BB.getTerminator()); // Insert BEFORE the final statement
      std::vector<Constant*> indices;
      Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(*Context));
      indices.push_back(zero);
      indices.push_back(zero);
      Constant *var_ref = ConstantExpr::getGetElementPtr(var, indices);
 
      Value *bbc = builder.CreateLoad(bbCounter);
      CallInst *call = builder.CreateCall2(printf_func, var_ref, bbc);
      call->setTailCall(false);
    }
  };
}
char CS201PathProfiling::ID = 0;
static RegisterPass<CS201PathProfiling> X("pathProfiling", "CS201PathProfiling Pass", false, false);
 
