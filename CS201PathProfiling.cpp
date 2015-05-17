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
#include <stack> 
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
//	std::vector<BasicBlock*> dominators; //new std::vector<BasicBlock*>;
	vector<pair<BasicBlock*, std::vector<BasicBlock*> > > dompairs;
	vector<pair<BasicBlock*, BasicBlock*>> back_edge;
	vector<vector<BasicBlock*>> loops;
	
	static bool myfunction(BasicBlock* i, BasicBlock* j)
	{
		string namei = i->getName();
		string namej = j->getName();
		int first = atoi(namei.c_str());
		int second = atoi(namej.c_str());
		return (first < second);
	}
	





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

	//dominate tree
	errs() << "dominator tree \n";
	for(unsigned i = 0; i < dompairs.size(); i++)
	{
		errs() << dompairs[i].first->getName() << ": ";
		for(unsigned j = 0; j < dompairs[i].second.size();j++)
		{		
			errs() << dompairs[i].second[j]->getName() << ", ";
		}
		errs() << "\n";
	} 
	errs() << "end dominator tree \n" ;

	

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
	eliminate();
	find_back_edge();
	eliminate_back_edge();
	find_loops();
	errs() << "\n";
	/*
	for(vector<BasicBlock*>::iterator it = back_edge.begin(); it != back_edge.end();it++)
	{
		errs() << it.first << " -> " <<it.second << "\t";
	}
	*/
	for(unsigned i = 0; i < back_edge.size();i++)
	{
		errs() << back_edge[i].first->getName() << " -> " <<back_edge[i].second->getName() << "\t";
	}
	
 
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
		vector<BasicBlock*> BBsTemp = BBs;
		BBsTemp.erase(BBsTemp.begin());
		vector<BasicBlock*> BBsInit;
		BBsInit.push_back(BBs[0]); 
		dompairs.push_back(make_pair(BBs[0],BBsInit));
/*
//begin print
		errs() << dompairs[0].first->getName() << ": ";
		j = 0;
		while(j < dompairs[0].second.size()){
			errs() << ((dompairs[0].second)[j])->getName() <<", ";
			j++;
		}
		errs() << "\n";
//end print
*/
		while(i < size)
		{
			vector<BasicBlock*> BBsT = BBs;
			
			dompairs.push_back(std::make_pair(BBs[i],BBsT));

//begin print		
//			errs() << dompairs[i].first->getName() << ": ";
//			j = 0;
//			while(j < dompairs[i].second.size()){
//				errs() << ((dompairs[i].second)[j])->getName() <<", ";
//				j++;
//			}
//			errs() << "\n";
//end print			
			i++;
		}
		// end initialization

		bool change = true;
		unsigned cnt;
		while(change)
		{
			change = false;
			for(cnt = 1; cnt < dompairs.size();cnt++)
			{
				vector<BasicBlock*> newdoms;
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
					vector<BasicBlock*> temptemp;
					std::set_intersection(temp.begin(), temp.end(),(dompairs[domNum].second).begin(), (dompairs[domNum].second).end(), inserter(temptemp,temptemp.begin()));
					temp = temptemp;
					
				}
				bool insert = true;
				for(auto *B : temp)
				{
					if(B->getName() == dompairs[cnt].first->getName()){
						insert = false;
						break; 
					}
				}
				newdoms = temp;

				if(insert)
					newdoms.push_back(dompairs[cnt].first);
	
			//sort
				std::sort(newdoms.begin(), newdoms.end(),myfunction);

				if(!equal(newdoms.begin(), newdoms.end(), dompairs[cnt].second.begin()))
				{
					change = true;	
				} 
				dompairs[cnt].second = newdoms;

//begin print
/*
				errs() << "it's temp:   \n";
				for(auto *B : temp)
					errs() << B->getName() << ", ";				
*/
//end print				
			}
		}
	}
	void find_back_edge()
	{
		unsigned cnt;
		for(cnt = 0; cnt < dompairs.size();cnt++)
		{
			int i;
			for(i = 1; i < (dompairs[cnt].second).size();i++)
			{
				BasicBlock* Bb = dompairs[cnt].second[i];
				vector<BasicBlock*>::iterator it;
				for(succ_iterator PI = succ_begin(Bb),E=succ_end(Bb); PI != E; ++PI)
				{
					BasicBlock *succ = *PI;
					int j;
					for(j = 0;j<i;j++)
					{
						if(succ->getName() == (dompairs[cnt].second[j])->getName())
						{
							back_edge.push_back(std::make_pair(dompairs[cnt].second[i],dompairs[cnt].second[j]));
							
						}
					}
				}
				
			}
		}
	}

	void eliminate()
	{
		if(dompairs.size() != BBs.size())
		{
			unsigned size_bbs = BBs.size();
			unsigned size_dom = dompairs.size();
			dompairs.erase(dompairs.begin() + size_bbs - 1, dompairs.end());
		}
	}
	void eliminate_back_edge()
	{
		bool change = true;
		bool jump = false;
		while(change){
			change = false;
			for(unsigned i=0; i < back_edge.size();i++)
			{
				for(unsigned j=i+1;j<back_edge.size();j++)
				{
					if(back_edge[i].first->getName() == back_edge[j].first->getName() 
						&& back_edge[i].second->getName() == back_edge[j].second->getName())
					{
						back_edge.erase(back_edge.begin() + i);
						//jump = true;
						change = true;
						break;
					}
	
				}
			//	if(jump)
			//	{
			//		jump = false;
			//		break;
			//	}
			}
		}	
	}
	void find_loops()
	{
		for(unsigned i = 0; i<back_edge.size();i++)
		{
			find_loop(back_edge[i].first, back_edge[i].second);
		}
	}
	void find_loop(BasicBlock *n, BasicBlock *d)
	{
		vector<BasicBlock*> loop;
		//initial
		stack<BasicBlock*> stacks;
		loop.push_back(d);
		loop.push_back(n);
		stacks.push(n);	
		
		while(!stacks.empty())
		{
			BasicBlock* m = stacks.top();
			stacks.pop();
			for(pred_iterator PI = pred_begin(m),E=pred_end(m);PI != E;++PI)
			{
				BasicBlock *pred = *PI;
				bool isThere = false;
				for(unsigned i = 0; i<loop.size();i++)
				{
					if(pred->getName() == m->getName())
						isThere = true;
						break; 
				}
				if(!isThere)
				{
					loop.push_back(pred);
					stacks.push(pred);
					isThere = false;
				}
			}
		}	
		
		loops.push_back(loop);
			

	}
	
	
/*
	vector<BasicBlock*> set_union(vector<BasicBlock*> fst, vector<BasicBlock*> snd)
	{
		vector<BasicBlock*> newOne;
		newOne = fst;
		for(auto *B : snd)
			newOne.push_back(B);
		unsigned i = 0;
		bool change = true;
		while(change)
		{
			change = false;
			for(i = 0; i < newOne.size();i++)
			{
				unsigned j = i+1;
				for(j =i+1;j< newOne.size();j++)
					if(newOne[i] == newOne[j])
					{
						change = true;
						newOne.erase(newOne[i]);
						continue;			
					}
				
			}
		}	 
	}
*/
 
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
 
