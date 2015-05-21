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
	static Function* malloc_prototype(LLVMContext& ctx, Module *mod) {
		std::vector<Type*> malloc_arg_types;
		malloc_arg_types.push_back(Type::getInt64Ty(ctx));

		PointerType *ret = Type::getInt8PtrTy(ctx);
		FunctionType* malloc_type = FunctionType::get(ret, malloc_arg_types, false);
		Function *func = mod->getFunction("malloc");
		if(!func)
			func = Function::Create(malloc_type, Function::ExternalLinkage, Twine("malloc"), mod);
		func->setCallingConv(CallingConv::C);
		return func;
	}

  
  struct CS201PathProfiling : public FunctionPass {
    static char ID;
    LLVMContext *Context;
    CS201PathProfiling() : FunctionPass(ID) {}
    GlobalVariable *bbCounter = NULL;
    Function *printf_func = NULL;

	GlobalVariable *r = NULL;
	GlobalVariable *counter = NULL;
	Function *malloc_func = NULL;
	int arraySize;
	int offset_index;

	GlobalVariable *formatStr0 = NULL;
	GlobalVariable *formatStr1 = NULL;
	GlobalVariable *formatStr2 = NULL;
	GlobalVariable *BasicBlockPrintfFormatStr = NULL;
	vector<long> entry;
	vector<int> path_num;

	vector<int> offset = {0};

	struct edge {
		BasicBlock * from;
		BasicBlock * to;
		int val;	
	};

	struct loop_path{
		vector<BasicBlock*> loop;
		int num_path;
		pair<BasicBlock*,BasicBlock*> back_edge;
	};
		
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

	r = new GlobalVariable(M, Type::getInt32Ty(*Context), false, GlobalValue::InternalLinkage, ConstantInt::get(Type::getInt32Ty(*Context), 0), "r");
	
	counter = new GlobalVariable(M, Type::getInt32PtrTy(*Context), false, GlobalValue::ExternalLinkage, Constant::getNullValue(Type::getInt32PtrTy(*Context)),"count");
	counter->setAlignment(8);

	const char *str0 = "Function %d:\n";
		formatStr0 = new GlobalVariable(M,llvm::ArrayType::get(llvm::IntegerType::get(*Context, 8), strlen(str0)+1),true, llvm::GlobalValue::PrivateLinkage, ConstantDataArray::getString(*Context, str0),"formatStr0");
	const char *str1 = "  Loop 0x%x:\n";
	formatStr1 =new GlobalVariable(	M, llvm::ArrayType::get(llvm::IntegerType::get(*Context, 8), strlen(str1)+1), true, llvm::GlobalValue::PrivateLinkage, ConstantDataArray::getString(*Context, str1),"formatStr1");

	const char *str2 = "    Path 0x%x %d: %d\n";formatStr2 = new GlobalVariable(	M, llvm::ArrayType::get(llvm::IntegerType::get(*Context, 8), strlen(str2)+1), true,llvm::GlobalValue::PrivateLinkage, ConstantDataArray::getString(*Context, str2),"formatStr2");

	printf_func = printf_prototype(*Context, &M);

	malloc_func = malloc_prototype(*Context, &M);
	arraySize = 0;
	offset_index = 0;

      return true;
    }
 
    //----------------------------------
    bool doFinalization(Module &M) {
      errs() << "-------Finished CS201PathProfiling----------\n";
	for(auto &F : M)
	{
		for(auto &BB : F)
		{
			char str[20];
			sprintf(str,"%p",&BB);
			BB.setName("BasicBlock_"+BB.getName() + " : "+str);
		}
	}
	errs() << M;	

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
	vector<vector<edge>> edges;
	vector<int> paths;
	vector<pair<BasicBlock*,BasicBlock*>> inner_back_edges;
	vector<loop_path> loops;

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
	
//	errs()<<"\n\n\noffset first value: "<<offset[0]<<"   size = " << offset.size()<<"\n";
	
	for(unsigned i = 0;i<inner_loops.size();i++)
	{
		inner_back_edges.push_back(make_pair(inner_loops[i][1],inner_loops[i][0]));
	}

	
	for(unsigned i=0;i<inner_loops.size();i++){
		
		paths.push_back(calTotalPath(inner_back_edges[i],inner_loops[i]));
		long address = (long)back_edge[i].second;
		for(unsigned j=0;j<paths[i];j++){
			entry.push_back(address);
			path_num.push_back(int(j));
		}

	//	errs() << "arraysizs is " << paths[i] <<"\n";
	}
	
	int sum=0;
	for(unsigned i=0;i<paths.size();i++)
	{
		loop_path temp;
		temp.loop = inner_loops[i];
		temp.num_path = paths[i];
		temp.back_edge=make_pair((inner_loops[i])[1],(inner_loops[i])[0]);
		loops.push_back(temp);	
			
		sum += paths[i];
	}
	arraySize += sum;
//	errs()<<"array size is : " << arraySize<<"\n";
	if(paths.size() != 0)
//	errs()<<"\n\npaths:  "<< paths[0] <<"\n\n";

	if(paths.size() != 0)
	for(unsigned i=0;i<paths.size();i++)
	{
		offset.push_back(paths[i] + offset[offset.size()-1]);
			}

//	errs() <<"\n\nofsset::\n";
/*
	for(unsigned i=0;i<offset.size();i++)
	{
		errs() <<offset[i] <<", ";
	}
	errs() <<"\n";
*/	
//	errs() << "\n\ngot here\n\n";
	
	//if(inner_loops.size() != 0)
	for(unsigned i= 0; i<inner_loops.size();i++)
	{
		toplos.push_back(topological_order(inner_loops[i],true));
		assign_value(toplos[i],F,offset_index,offset);
		offset_index++;
	}
	
			
//	print_dominator(dompairs);
//	print_back_edge(back_edge);
	print_loop(inner_loops);


//	errs() << "add blocks:  \n";
//	if(BBs.size() > 1)
//		add_block(BBs[0],BBs[1], F);
/*
	for(auto &B: F)
	{
		errs() << B.getName() << ", ";
	}
*/
//	errs() << "\nend adding....";
	if(F.getName().equals("main"))
	{
		addMalloc(F.getEntryBlock(),Context,malloc_func);
		for (auto &BB: F)
			if (isa<ReturnInst>(BB.getTerminator()))
			{
				for (int i=0;i<arraySize;i++)
					addPrintf(BB,Context,counter,formatStr2,printf_func,entry[i],path_num[i],i);
			}
	}

	errs() << "\n";


      return true; // since runOnBasicBlock has modified the program
    }
 
    //----------------------------------
    bool runOnBasicBlock(BasicBlock &BB) {
      errs() << "BasicBlock: " << BB.getName() << "    "<<&BB<<'\n';
      IRBuilder<> IRB(BB.getFirstInsertionPt()); // Will insert the generated instructions BEFORE the first BB instruction
 
      Value *loadAddr = IRB.CreateLoad(bbCounter);
      Value *addAddr = IRB.CreateAdd(ConstantInt::get(Type::getInt32Ty(*Context), 1), loadAddr);
      IRB.CreateStore(addAddr, bbCounter);
 
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

	}

	bool in_list(unsigned i, vector<unsigned> &list)
	{
		for(unsigned j = 0; j<list.size();j++)
			if(i == list[j])
				return true;
		return false;
	}

	vector<BasicBlock*> topological_order(vector<BasicBlock*> &bbs, bool is_loop)
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
	//	print_temp_list(temp_list);
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
						for(unsigned j=0;j<temp_list.size();j++)
						{
							if(pred->getName() == temp_list[j].first->getName()){
								temp_list[j].second = temp_list[j].second - 1;
							}
						}	
					}
					break;	
				}
			}
		}
	//		errs() << "topological order: \n";
		return rev_topolog;
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

	void assign_value(vector<BasicBlock*> &rev_topology, Function &F,int index, vector<int> &off_set)
	{
		vector<edge> edges;
		edges_obtain(edges, rev_topology);
		vector<pair<BasicBlock*, int>> numPaths;
		unsigned size_top = rev_topology.size();
		
		for(unsigned i = 0; i < rev_topology.size(); i++)
			numPaths.push_back(make_pair(rev_topology[i],0));
		
		for(unsigned i = 0; i<numPaths.size(); i++)
		{	
			
			if(i == 0)
				numPaths[i].second = 1;
			else
			{
				numPaths[i].second = 0;
				vector<edge*> bb_edges;
				get_bb_edges(numPaths[i].first, bb_edges, edges);
				for(unsigned j=0;j<bb_edges.size();j++)
				{
					edge *e = bb_edges[j];
					e->val = numPaths[i].second;
					numPaths[i].second = numPaths[i].second + num_of_path(e, numPaths);
				}
			} 
		}

	//		errs() <<"\n\n+++++ 645+++  index = "<<index <<", "<<off_set[index] <<"\n\n";
			add_block_back_edge(rev_topology[0],rev_topology[size_top-1],F,off_set[index]);
			
		
		for(unsigned i = 0;i<edges.size();i++)
		{
			add_block(edges[i].from, edges[i].to, F,edges[i].val);
		}

		//errs() << "insert in: "<< rev_topology[rev_topology.size()-1]->getName()<<'\n';
		IRBuilder<> builder(rev_topology[rev_topology.size()-1]->getFirstInsertionPt());
		builder.CreateStore(ConstantInt::get(Type::getInt32Ty(*Context),0),r);
		

		//print_edges(edges);
		//print_num_path(numPaths);
			
	}
	int num_of_path(edge* e, vector<pair<BasicBlock*,int>> &numPaths)
	{
		for(unsigned i = 0; i<numPaths.size();i++)
		{
			if(e->to->getName() == numPaths[i].first->getName())
				return numPaths[i].second;
		}
		return 0;
	}

   void addMalloc(BasicBlock& BB,LLVMContext *Context,Function *malloc_func)
	{
		int size = arraySize;
	//	errs() << "in malloc\n";
		//count=(int*)malloc(size);
		IRBuilder<> builder(BB.getFirstInsertionPt());
		Constant *arg = ConstantInt::get(Type::getInt64Ty(*Context),size*sizeof(int));
		CallInst *call = builder.CreateCall(malloc_func, arg);
		Value *cast = builder.CreateBitCast(call, Type::getInt32PtrTy(*Context));
		builder.CreateStore(cast,counter);
		//count[i]=0;
		for (int i=0;i<size;i++)
		{
			Value *cnt = builder.CreateLoad(counter);
			std::vector<Value*> list;
			list.push_back(ConstantInt::get(Type::getInt64Ty(*Context),i));
			Value *ele = GetElementPtrInst::Create(cnt,list,"",builder.GetInsertPoint());
			builder.CreateStore(ConstantInt::get(Type::getInt32Ty(*Context),0),ele);
		}
	}
	void get_bb_edges(BasicBlock * bb, vector<edge*> &bb_edges, vector<edge> &edges)
	{
		for(unsigned i=0; i<edges.size(); i++)
		{
			if(bb->getName() == (edges[i].from)->getName())
			{	
				edge * ptr = &edges[i];
				bb_edges.push_back(ptr);
			}
		}
	}

	void edges_obtain(vector<edge> &edges, vector<BasicBlock*> &rev_toplogy)
	{
		
		for(unsigned i=1; i < rev_toplogy.size(); i++)
		{
			BasicBlock* Bb = rev_toplogy[i];
			for(succ_iterator PI = succ_begin(Bb),E=succ_end(Bb); PI != E; ++PI)
			{
				BasicBlock* s = *PI;
				if(in_BB_list(s, rev_toplogy))
				{
					edge e;
					e.from = Bb; 
					e.to = s;
					e.val = 0;
					edges.push_back(e);
				}
			}	
		}
		
	}


	void add_block(BasicBlock* a, BasicBlock* b, Function &F,int val)
	{
		IRBuilder<> *irb = new IRBuilder<>(*Context);
		int count = 0;
		for(succ_iterator SI = succ_begin(a),E=succ_end(a);SI != E;++SI)
		{
			if(b != *SI) continue;

			BasicBlock* add = BasicBlock::Create(*Context,"",&F,b);
			add->setName("add");
			a->getTerminator()->setSuccessor(count, add);
			count++;

			irb->SetInsertPoint(add);
		//	if(a==exit && b==entry){
			irb->CreateBr(b);

			IRBuilder<> builder(add->getTerminator());
			Value *load = builder.CreateLoad(r);
			Value *addAddr = builder.CreateAdd(ConstantInt::get(Type::getInt32Ty(*Context),val),load);
			builder.CreateStore(addAddr, r);
				//IRcntradd(irb,add);
		//	}
		//	else
				//IRradd(irb,a,b,1)
			
		}	
	}

	void add_block_back_edge(BasicBlock* a, BasicBlock* b, Function &F,int off_set)
	{
		IRBuilder<> *irb = new IRBuilder<>(*Context);
		int count = 0;

		for(succ_iterator SI = succ_begin(a),E=succ_end(a);SI != E;++SI)
		{
			if(b != *SI) continue;

			BasicBlock* add = BasicBlock::Create(*Context,"",&F,b);
			add->setName("add");
			a->getTerminator()->setSuccessor(count, add);
			count++;

			irb->SetInsertPoint(add);
			irb->CreateBr(b);

			IRBuilder<> builder(add->getTerminator());
			Value *load_r = builder.CreateLoad(r);
			Value *cnt = builder.CreateLoad(counter);

//--------------->>>>>change '0' value
			Value *addAddr = builder.CreateAdd(ConstantInt::get(Type::getInt32Ty(*Context),off_set),load_r);
	//		errs() << "\n\noffset: "<<off_set<<"\n\n";
			vector<Value*> list;
			list.push_back(addAddr);
			Value *ele = GetElementPtrInst::Create(cnt, list, "",add->getTerminator());
			Value *op = builder.CreateLoad(ele);
			Value *ddd = builder.CreateAdd(ConstantInt::get(Type::getInt32Ty(*Context),1),op);		
	
			builder.CreateStore(ddd, ele);
//------------>>>>>>print insert
		//	addPrintf(*add,Context,counter,formatStr2,printf_func,1,r+off_set);
		}	
	}
	int calTotalPath(pair<BasicBlock*,BasicBlock*> &backedge,
							vector<BasicBlock*> &loop)
		{
			int totalPath=0;
			std::stack<BasicBlock*> st;
			st.push(backedge.second);
			
			while (!st.empty())
			{
				BasicBlock *x=st.top();
				st.pop();
				
				if (x==backedge.first) 
					totalPath++;
				else
					for (succ_iterator it=succ_begin(x);it!=succ_end(x);it++)
						if (find_in(loop, *it))
							st.push(*it);
			}
			
			
			return totalPath;	
		}
			
	bool find_in(vector<BasicBlock*> &list, BasicBlock* bb)
	{
		for(unsigned i = 0; i<list.size();i++)
		{
			if(list[i] == bb)
				return true;
		}	
		return false;
	}
	
	bool in_BB_list(BasicBlock* block, vector<BasicBlock*> &blocks)
	{
		for(unsigned i = 0; i < blocks.size(); i++)
		{
			if(block->getName() == blocks[i]->getName())
				return true;
		}
		return false;
	}

	void print_edges(vector<edge> &edges)
	{
		errs() << "edges --- >>>> :\n";
		for(unsigned i = 0; i < edges.size(); i++)
		{		
			errs() << "[" << edges[i].from->getName()
			<<"] -> [" << edges[i].to->getName() <<"] value: "
			<< edges[i].val <<"\n";
		}
		errs() << "end edges ------<<<<<<\n";
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
	void print_num_path(vector<pair<BasicBlock*,int>> &numPaths)
	{
		errs() << "num paths: --->>>>\n";
		for(unsigned i=0;i<numPaths.size();i++)
		{
			errs() << "block: ["<<numPaths[i].first->getName()
				<< "], value: ["<<numPaths[i].second<<"]\n";
		}
	}
	
	void addPrintf(		BasicBlock& BB, 
							LLVMContext *Context, 
							GlobalVariable *str, 
							Function *printf_func,
							BasicBlock *ptr)
		{
			//printf(str,ptr);
			IRBuilder<> builder(BB.getTerminator()); 
			std::vector<Constant*> indices;
			Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(*Context));
			indices.push_back(zero);
			indices.push_back(zero);
			Constant *arg = ConstantExpr::getGetElementPtr(str, indices);
	
			long ptrVal = long(ptr);
			Value *tmp = (ConstantInt::get(Type::getInt64Ty(*Context),ptrVal));
			CallInst *call = builder.CreateCall2(printf_func, arg, tmp);
			call->setTailCall(false);
		}


		void addPrintf(BasicBlock& BB,LLVMContext *Context,GlobalVariable *count, GlobalVariable *str, Function *printf_func,long ptr,int num,int index) 
		{
			//printf(str,ptr,num,count[index]);
			IRBuilder<> builder(BB.getTerminator()); 
			std::vector<Constant*> indices;
			Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(*Context));
			indices.push_back(zero);
			indices.push_back(zero);
			Constant *arg = ConstantExpr::getGetElementPtr(str, indices);
	
			Value *cnt = builder.CreateLoad(count);
			std::vector<Value*> list;
			list.push_back(ConstantInt::get(Type::getInt64Ty(*Context),index));
			Value *ele = GetElementPtrInst::Create(cnt,list,"",BB.getTerminator());
			Value *tmp = builder.CreateLoad(ele);
			Value *tmp2= (ConstantInt::get(Type::getInt64Ty(*Context),ptr));
			Value *tmp3= (ConstantInt::get(Type::getInt64Ty(*Context),num));
			CallInst *call = builder.CreateCall4(printf_func, arg, tmp2, tmp3, tmp);
			call->setTailCall(false);
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
 
