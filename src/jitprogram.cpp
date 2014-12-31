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
	namespace compile{
		class GlobalContext{
		public:
			llvm::Module *module;
			llvm::ExecutionEngine *jit;

			GlobalContext(){
				LLVMInitializeNativeTarget();
				module=new llvm::Module("loglang", llvm::getGlobalContext());
				jit = llvm::EngineBuilder(module).create();
			}
		};
		
		GlobalContext context; // Global LLVM context
	}
	class CompileContext{
	public:
		std::stack<llvm::BasicBlock*> stack;
		Context &context; // Loglng context, for globals.
		
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
	llvm_function=llvm::Function::Create(FT, llvm::Function::ExternalLinkage, program_name, compile::context.module);

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
	pm.run(*loglang::compile::context.module);
	std::cout<<std::endl;
}


void JITProgram::run(){
	std::cout<<"Run"<<std::endl;
	std::vector<llvm::GenericValue> args;
	if (!llvm_function)
		throw std::runtime_error("Function not compiled yet.");
	compile::context.jit->runFunction(llvm_function, args);
}

namespace loglang{
namespace ast{
	llvm::Value *Assign::compile(CompileContext &context){
		//return new StoreInst(rhs, lhs, false, context.getCurrentBlock());;
		throw compile_error("Not yet Assign");
	}
	llvm::Value *Edge_if::compile(CompileContext &context){
		throw compile_error("Not yet edge_if");
	}
	llvm::Value *Value_var::compile(CompileContext &context){
		return context.context.get_value(var).llvm_value(compile::context.module);
	}
	llvm::Value *Value_const::compile(CompileContext &context){
		llvm::Type *typ;
		if (val->type_name=="double")
			return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), val->to_double());
		if (val->type_name=="int")
			return llvm::ConstantFP::get(llvm::Type::getInt64PtrTy(llvm::getGlobalContext()), val->to_int());
		throw std::runtime_error("There is no support for this type yet.");
	}
	llvm::Value *Function::compile(CompileContext &context){
		throw compile_error("Not yet Function");
	}
	llvm::Value *Expr_div::compile(CompileContext &context){
		throw compile_error("Not yet Div");
	}
	llvm::Value *Expr_mul::compile(CompileContext &context){
		throw compile_error("Not yet Mul");
	}
	llvm::Value *Expr_add::compile(CompileContext &context){
		throw compile_error("Not yet Add");
	}
	llvm::Value *Expr_sub::compile(CompileContext &context){
		throw compile_error("Not yet Sub");
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
	llvm::Value *Expr_Expr::compile(CompileContext &context){
		op1->compile(context);
		return op2->compile(context);
	}

}
}
