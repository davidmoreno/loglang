/*
 * Copyright 2014 David Moreno
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <stack>

#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>

#include "ast.hpp"
#include "context.hpp"
#include "ast_all.hpp"
#include "jitprogram.hpp"


using namespace loglang;

namespace loglang {
	namespace jit{
		RuntimeContext::RuntimeContext(){
			LLVMInitializeNativeTarget();
			module=new llvm::Module("loglang", llvm::getGlobalContext());
			jit = llvm::EngineBuilder(module).create();
			
			
			set_val_f = module->getFunction("set_val_f");
			{
				/// Prepare set_val_f.
				llvm::PointerType* PointerTy_5 = llvm::PointerType::get(llvm::IntegerType::get(module->getContext(), 8), 0);
				std::vector<llvm::Type*>FuncTy_7_args;
				FuncTy_7_args.push_back(PointerTy_5);
				FuncTy_7_args.push_back(PointerTy_5);
				FuncTy_7_args.push_back(llvm::Type::getFloatTy(module->getContext()));
				llvm::FunctionType* FuncTy_7 = llvm::FunctionType::get(
					/*Result=*/llvm::Type::getVoidTy(module->getContext()),
					/*Params=*/FuncTy_7_args,
					/*isVarArg=*/false);

				if (!set_val_f) {
				set_val_f = llvm::Function::Create(
					/*Type=*/FuncTy_7,
					/*Linkage=*/llvm::GlobalValue::ExternalLinkage,
					/*Name=*/"set_val_f", module); // (external, no body)
				set_val_f->setCallingConv(llvm::CallingConv::C);
				}
				llvm::AttributeSet func_set_val_f_PAL;
				{
					llvm::SmallVector<llvm::AttributeSet, 4> Attrs;
					llvm::AttributeSet PAS;
					{
						llvm::AttrBuilder B;
						PAS = llvm::AttributeSet::get(module->getContext(), ~0U, B);
					}
					
					Attrs.push_back(PAS);
					func_set_val_f_PAL = llvm::AttributeSet::get(module->getContext(), Attrs);
					
				}
				set_val_f->setAttributes(func_set_val_f_PAL);
			}

// 			{
// 				llvm::Type *type=llvm::PointerType::get(llvm::IntegerType::get(module->getContext(), 8), 0);;
// 				module->getOrInsertGlobal("__runtime_context__", type);
// 				context_value=module->getGlobalVariable("__runtime_context__");
// 				context_value->setLinkage(llvm::GlobalValue::InternalLinkage);
// 				llvm::Constant* ctx_ptr=llvm::ConstantInt::get(module->getContext(), llvm::APInt(64,(uint64_t)this));
// 
// 				context_value->setInitializer(ctx_ptr);
// 			}
// 			std::cerr<<"ctx is at "<<this<<std::endl;
		}
		
		RuntimeContext context; // Global LLVM context
	}
	class CompileContext{
	public:
		std::stack<llvm::BasicBlock*> stack;
		::loglang::Context &context; // Loglng context, for globals.
		
		CompileContext(llvm::BasicBlock *bblock, Context &_context) : context(_context){
			stack.push(bblock);
		}
		
		llvm::BasicBlock *currentBlock(){
			return stack.top();
		}
	};
}

JITProgram::JITProgram(const std::string &program_name, const AST& root_node, Context &ctx){

	llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);	
	llvm_function=llvm::Function::Create(FT, llvm::Function::ExternalLinkage, program_name, jit::context.module);

	llvm::BasicBlock *bblock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", llvm_function);

	CompileContext cc(bblock, ctx);

	llvm::Value *root=root_node->compile(cc);
	
	llvm::ReturnInst::Create(llvm::getGlobalContext(), nullptr, bblock);

	showCode();
}

JITProgram::~JITProgram()
{ 
	// FIXME! Remove all data for this function.
}

void JITProgram::showCode()
{
	std::cout << "Code generated.\n";
	llvm::PassManager pm;
	pm.add(llvm::createPrintModulePass(llvm::outs()));
	pm.run(*loglang::jit::context.module);
	std::cout<<std::endl;
}


void JITProgram::run(){
// 	std::cout<<"Run"<<std::endl;
	std::vector<llvm::GenericValue> args;
	if (!llvm_function)
		throw std::runtime_error("Function not compiled yet.");
	jit::context.jit->runFunction(llvm_function, args);
// 	std::cout<<"Done "<<std::endl;
}

namespace loglang{
namespace ast{
	llvm::Value *Assign::compile(CompileContext &context){
		//return new StoreInst(rhs, lhs, false, context.getCurrentBlock());;
		Symbol &var=context.context.get_value(this->var);
		llvm::Value *value=var.llvm_value();
		llvm::Value *new_val=op2->compile(context);
		llvm::Value *ret=new llvm::StoreInst(
			new_val, 
			value, 
			false, context.currentBlock());
		
// 		std::cerr<<(void*)&context.context<<std::endl;
		llvm::Value *context_ptr=llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), (int64_t)&context.context);
		llvm::Value *var_name=var.llvm_value_name();
		std::vector<llvm::Value*> set_val_f_params;
		set_val_f_params.push_back(context_ptr);
		set_val_f_params.push_back(var_name);
		set_val_f_params.push_back(new_val);
		llvm::CallInst* set_val_f_call = llvm::CallInst::Create(jit::context.set_val_f, set_val_f_params, "",  context.currentBlock());
		set_val_f_call->setCallingConv(llvm::CallingConv::C);
		set_val_f_call->setTailCall(false);
		
		
// 		llvm::AttributeSet void_21_PAL;
// 		set_val_f_call->setAttributes(void_21_PAL);

	}
	llvm::Value *Edge_if::compile(CompileContext &context){
		throw compile_error("Not yet edge_if");
	}
	llvm::Value *Value_var::compile(CompileContext &context){
		return new llvm::LoadInst(context.context.get_value(var).llvm_value(), "", false, context.currentBlock());
	}
	llvm::Value *Value_const::compile(CompileContext &context){
// 		std::cerr<<"Type  "<<val->type_name<<std::endl;
		if (val->type_name=="double")
			return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), val->to_double());
		if (val->type_name=="int")
			return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), val->to_int());
		throw compile_error("There is no support for this type yet.");
	}
	llvm::Value *Function::compile(CompileContext &context){
		throw compile_error("Not yet Function");
	}
	llvm::Value *Expr_div::compile(CompileContext &context){
		llvm::Value *a=op1->compile(context);
		llvm::Value *b=op2->compile(context);
		return llvm::BinaryOperator::Create(llvm::Instruction::FDiv, a, b, "", context.currentBlock());
	}
	llvm::Value *Expr_mul::compile(CompileContext &context){
		llvm::Value *a=op1->compile(context);
		llvm::Value *b=op2->compile(context);
		return llvm::BinaryOperator::Create(llvm::Instruction::FMul, a, b, "", context.currentBlock());
	}
	llvm::Value *Expr_add::compile(CompileContext &context){
		llvm::Value *a=op1->compile(context);
		llvm::Value *b=op2->compile(context);
		return llvm::BinaryOperator::Create(llvm::Instruction::FAdd, a, b, "", context.currentBlock());
	}
	llvm::Value *Expr_sub::compile(CompileContext &context){
		llvm::Value *a=op1->compile(context);
		llvm::Value *b=op2->compile(context);
		return llvm::BinaryOperator::Create(llvm::Instruction::FSub, a, b, "", context.currentBlock());
	}
	llvm::Value *At::compile(CompileContext &context){
		throw compile_error("Not yet At");
	}
	llvm::Value *Value_glob::compile(CompileContext &context){
		throw compile_error("Not yet Val glob");
	}
	llvm::Value *Expr_eq::compile(CompileContext &context){
		return llvm::CmpInst::Create(llvm::Instruction::FCmp, llvm::CmpInst::ICMP_EQ, op1->compile(context), op2->compile(context), "", context.currentBlock());
	}
	llvm::Value *Expr_neq::compile(CompileContext &context){
		throw compile_error("Not yet neq");
	}
	llvm::Value *Expr_lt::compile(CompileContext &context){
		throw compile_error("Not yet lt");
	}
	llvm::Value *Expr_lte::compile(CompileContext &context){
		throw compile_error("Not yet lte");
	}
	llvm::Value *Expr_gt::compile(CompileContext &context){
		throw compile_error("Not yet gt");
	}
	llvm::Value *Expr_gte::compile(CompileContext &context){
		throw compile_error("Not yet gte");
	}
	llvm::Value *Expr_or::compile(CompileContext &context){
		throw compile_error("Not yet or");
	}
	llvm::Value *Expr_and::compile(CompileContext &context){
		throw compile_error("Not yet and");
	}
	llvm::Value *Block::compile(CompileContext &context){
		llvm::Value *ret=NULL;
		for(auto &st: stmts)
			ret=st->compile(context);
		return ret;
	}

}
}

extern "C"{
	void set_val_f(void *ctxv, const char *name, double v){
		loglang::Context *ctx=static_cast<loglang::Context*>(ctxv);
// 		std::cerr<<"Ctx "<<ctx<<" "<<name<<" "<<v<<std::endl;
		ctx->get_value(name).set(to_any(v), *ctx);
	}
}
