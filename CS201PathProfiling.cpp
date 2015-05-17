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
		
	static bool myfunction(BasicBlock* i, BasicBlock* j)
	{
		string namei = i->getName();
		string namej = j->getName();
		int first = atoi(namei.c_str());
		int second = atoi(namej.c_str());
		return (first < second);
	}

	static bool func_loops(vector<BasicBlock*> &left,vector<BasicBlock*> &right)
	{
		return (left.size() < right.size());
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
/*
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
*/
      errs()<<"\n";

      return false;
    }
    
    //----------------------------------
    bool runOnFunction(Function &F) override {


	//declaration
	std::vector<BasicBlock*> BBs;
	vector<pair<BasicBlock*, std::vector<BasicBlock*> > > dompairs;
	vector<pair<BasicBlock*, BasicBlock*>> back_edge;
	vector<vector<BasicBlock*>> loops_dom;
	vector<vector<BasicBlock*>> inner_loops;
	vector<vector<BasicBlock*>> toplos;

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
	formDominator(BBs, dompairs);	
	eliminate(dompairs, BBs);
	find_back_edge(dompairs, back_edge);
	eliminate_back_edge(back_edge);
	find_loops(back_edge, loops_dom);
	find_inner_loops(inner_loops, loops_dom);
	
//	for(unsigned i=0;i<inner_loops.size();i++)
//		toplos.push_back(topological_order(inner_loops[i], true));
	if(inner_loops.size() != 0)
		topological_order(inner_loops[0], true);
	else
		errs() << "no loops\n";
	print_dominator(dompairs);
	print_back_edge(back_edge);
	print_loop(loops_dom);


	errs() << "\n";

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
	void formDominator(vector<BasicBlock*> &BBs, vector<pair<BasicBlock*,vector<BasicBlock*>>> &dompairs)
	{

		// initialize, 
		unsigned size = BBs.size();
		unsigned i = 1;
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
	void find_back_edge(vector<pair<BasicBlock*, std::vector<BasicBlock*>>> &dompairs, vector<pair<BasicBlock*,BasicBlock*>> &back_edge)

	{
		unsigned cnt;
		for(cnt = 0; cnt < dompairs.size();cnt++)
		{
			unsigned i;
			for(i = 1; i < (dompairs[cnt].second).size();i++)
			{
				BasicBlock* Bb = dompairs[cnt].second[i];
				vector<BasicBlock*>::iterator it;
				for(succ_iterator PI = succ_begin(Bb),E=succ_end(Bb); PI != E; ++PI)
				{
					BasicBlock *succ = *PI;
					unsigned j;
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

	void eliminate(vector<pair<BasicBlock*,vector<BasicBlock*>>> &dompairs, vector<BasicBlock*> &BBs)
	{
		if(dompairs.size() != BBs.size())
		{
			unsigned size_bbs = BBs.size();
			dompairs.erase(dompairs.begin() + size_bbs - 1, dompairs.end());
		}
	}
	void eliminate_back_edge(vector<pair<BasicBlock*,BasicBlock*>> &back_edge)
	{
		bool change = true;
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
			}
		}	
	}
	void find_loops(vector<pair<BasicBlock*, BasicBlock*>> &back_edge, vector<vector<BasicBlock*>> &loops_dom)
	{
		for(unsigned i = 0; i<back_edge.size();i++)
		{
			find_loop(back_edge[i].first, back_edge[i].second, loops_dom);
		}
	}
	void find_loop(BasicBlock *n, BasicBlock *d, vector<vector<BasicBlock*>> &loops_dom)
	{
		vector<BasicBlock*> loop_dom;
		//initial
		stack<BasicBlock*> stacks;
		loop_dom.push_back(d);
		loop_dom.push_back(n);
		stacks.push(n);	
		BasicBlock* m;		
		bool isThere = false;
	//	int count = 0;
		while(!stacks.empty()/* && count < 5 */)
		{
			m = stacks.top();
			stacks.pop();
//			errs() << m->getName() << ", ";
			for(pred_iterator PI = pred_begin(m),E=pred_end(m);PI != E;++PI)
			{
				BasicBlock *pred = *PI;
//				errs() << "pred: " << pred->getName() <<"\n";
				for(unsigned i = 0; i<loop_dom.size();i++)
				{
					if(pred->getName() == loop_dom[i]->getName())
						isThere = true;
				}
				if(!isThere)
				{
					loop_dom.push_back(pred);
					stacks.push(pred);
				}
//				errs() << "stacks size: " << stacks.size() << "\n";
				isThere = false;
			}
		errs() << "\n";
		}	
		
		loops_dom.push_back(loop_dom);
	}


	void find_inner_loops(vector<vector<BasicBlock*>> &inner_loops, vector<vector<BasicBlock*>> &loops_dom)
	{

		vector<unsigned> erase_list;
		vector<vector<BasicBlock*>> loops = loops_dom;
		vector<vector<BasicBlock*>> inners;
		if(loops.size() > 1)
		{
			for(unsigned i=0; i<loops.size();i++)
			{
				for(unsigned j = 0; j<loops.size();j++)
				{
					if(i == j)
						continue;
					
					vector<BasicBlock*> intersec;
					set_intersection(loops[i].begin(),loops[i].end(),loops[j].begin(),loops[j].end(),inserter(intersec,intersec.begin()));
		
				if(equal(intersec.begin(),intersec.end(),loops[i].begin()) && loops[i].size() < loops[j].size())
					{
						erase_list.push_back(j);
					}
				}
			}
			if(erase_list.size() != 0)
			{
				for(unsigned i = 0;i<loops.size();i++)
				{
					if(!in_list(i,erase_list))
					{
						inners.push_back(loops[i]);
					}
				}
			}
			else
				inners = loops;
		}	
		else
			inners = loops;
		
		inner_loops = inners;

//		errs() << "\n before inner function --->>>";
//		print_loop(loops);
//		errs() <<"\n finding inner loops -->>> \n";
//		print_loop(inners);
	}

	bool in_list(unsigned i, vector<unsigned> &list)
	{
		for(unsigned j = 0; j<list.size();j++)
			if(i == list[j])
				return true;
		return false;
	}

	void topological_order(vector<BasicBlock*> &bbs, bool is_loop)
	{
		vector<pair<BasicBlock*, int>> temp_list;
		vector<BasicBlock*> rev_topolog;
		for(unsigned i=0;i<bbs.size();i++)
		{
			temp_list.push_back(make_pair(bbs[i],out_links(bbs[i],bbs)));
		}
		if(is_loop)
		{
			temp_list[1].second = temp_list[1].second - 1;
		}
//print
//		check_preds(bbs);
//		check_succs(bbs);
//		errs() << "show here:\n";
		print_temp_list(temp_list);
		bool change = true;

		
		while(change && rev_topolog.size() < temp_list.size())
		{
			change = false;
			for(unsigned i=0;i<temp_list.size();i++)
			{
				if(temp_list[i].second == 0)
				{
					if(i == 0){
						rev_topolog.push_back(temp_list[i].first);
						break;
					}
					temp_list[i].second = temp_list[i].second - 1;
					change = true;
					BasicBlock* Bb = temp_list[i].first;
					rev_topolog.push_back(Bb);
					for(pred_iterator PI = pred_begin(Bb),E=pred_end(Bb); PI != E; ++PI)
					{
						BasicBlock * pred = *PI;
//						errs() << "pred got here : " << pred->getName() << "\n";
						for(unsigned j=0;j<temp_list.size();j++)
						{
//							errs() << "comparing: "<<pred->getName()<<", "<<temp_list[j].first->getName() << "\n";
							if(pred->getName() == temp_list[j].first->getName()){
								temp_list[j].second = temp_list[j].second - 1;
//								errs() << temp_list[j].first->getName()<<" temp_list's second : " << temp_list[j].second <<"\n";
//								errs() << "very inner---->>>>>>>\n";
//								print_temp_list(temp_list);
							}
						}	
					}
//					errs() <<"inners::::::\n";
//					print_temp_list(temp_list);
					break;	
				}
			}
//			print_temp_list(temp_list);
		}
//print
//		print_temp_list(temp_list);
		errs() << "topological order: \n";
		print_BasicBlock(rev_topolog);
		

	}

	void check_preds(vector<BasicBlock*> &list)
	{
		errs() << "predecesors : \n";
		for(unsigned i=0; i<list.size(); i++)
		{
			errs() << list[i]->getName() << ": ";
			BasicBlock* Bb = list[i];
			for(pred_iterator PI = pred_begin(Bb),E=pred_end(Bb); PI != E; ++PI)
			{
				BasicBlock* pred = *PI;
				errs()<<pred->getName()<<", ";	 
			}
			errs() <<"\n";

		}
		errs() << "end preds.....\n";
	}
	void check_succs(vector<BasicBlock*> &list)
	{
		errs() << "successor: \n";
		for(unsigned i=0; i<list.size(); i++)
		{
			errs() << list[i]->getName() << ": ";
			BasicBlock* Bb = list[i];
			for(succ_iterator PI = succ_begin(Bb),E=succ_end(Bb); PI != E; ++PI)
			{
				BasicBlock* succ = *PI;
				errs()<< succ->getName()<<", ";	 
			}
			errs() <<"\n";

		}
		errs() << "end succs.....\n";
	}

	unsigned out_links(BasicBlock * bb, vector<BasicBlock*> &bbs)
	{
		unsigned count = 0;	
		for(succ_iterator PI = succ_begin(bb),E=succ_end(bb); PI != E; ++PI){
			BasicBlock* succ = *PI;
			for(unsigned i=0;i<bbs.size();i++)	
			{	
				if(succ->getName() == bbs[i]->getName()){
					count++;
				}
			}
		}
		return count;
	}

	
	void print_temp_list(vector<pair<BasicBlock*,int>> &list)
	{
		errs() <<"temp_list -- >> \n";
		for(unsigned i=0;i<list.size();i++)
		{
			errs() << list[i].first->getName() <<", ["
				<< list[i].second <<"]";
			errs() << "\n";
		}
		errs() << "\n";
	}

	void print_loop(vector<vector<BasicBlock*>> &loops_dom)
	{
		errs() <<"\n" <<"loops, size is  " << loops_dom.size()<<"\n";
		for(unsigned i = 0; i<loops_dom.size(); i++)
		{
			errs() << "loop: ";
			for(unsigned j = 0; j<loops_dom[i].size();j++)
			{
				errs()<<loops_dom[i][j]->getName() << ", ";
			}
			errs() << "\n";
		}
	}
	void print_back_edge(vector<pair<BasicBlock*, BasicBlock*>> &back_edge)
	{
		errs()<<"\nback edge: \n";
		for(unsigned i = 0; i<back_edge.size();i++)
		{		
			errs() << back_edge[i].first->getName()<< ", "
				<< back_edge[i].second->getName() <<"\n";
		}	
		errs() <<"\n";
	}
	void print_dominator(vector<pair<BasicBlock*,vector<BasicBlock*>>> &dompairs)
	{
		errs() <<"dominator tree \n" ;
		for(unsigned i = 0; i<dompairs.size();i++)
		{	
			errs() << dompairs[i].first->getName() <<": ";
			for(unsigned j = 0; j < dompairs[i].second.size();j++)
				errs() << dompairs[i].second[j]->getName() <<", ";
			errs() << "\n";
		}
		errs() << "\n";
	}
	void print_BasicBlock(vector<BasicBlock*> &list)
	{
		errs() << "list : ";
		for(unsigned i=0;i<list.size();i++)
			errs() << list[i]->getName() << ", ";
		errs() << "end list\n";
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
 
