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

#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/GenericValue.h>

#include "ast.hpp"
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
}

JITProgram::JITProgram(const std::string &program_name, const AST& root_node){
	llvm::Value *root=root_node->compile();

	llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);	
	llvm_function=llvm::Function::Create(FT, llvm::Function::ExternalLinkage, program_name, compile::context.module);

	llvm::BasicBlock *bblock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", llvm_function);
	
}

JITProgram::~JITProgram()
{ 
	// FIXME! Remove all data for this function.
}


void JITProgram::run(any&){
	std::cout<<"Run"<<std::endl;
	std::vector<llvm::GenericValue> args;
	compile::context.jit->runFunction(llvm_function, args);
}

namespace loglang{
namespace ast{
	llvm::Value *Equal::compile(){
		return nullptr;
	}
	llvm::Value *Edge_if::compile(){
		return nullptr;
	}
	llvm::Value *Value_var::compile(){
		return nullptr;
	}
	llvm::Value *Value_const::compile(){
		return nullptr;
	}
	llvm::Value *Function::compile(){
		return nullptr;
	}
	llvm::Value *Expr_div::compile(){
		return nullptr;
	}
	llvm::Value *Expr_mul::compile(){
		return nullptr;
	}
	llvm::Value *Expr_add::compile(){
		return nullptr;
	}
	llvm::Value *Expr_sub::compile(){
		return nullptr;
	}
	llvm::Value *At::compile(){
		return nullptr;
	}
	llvm::Value *Value_glob::compile(){
		return nullptr;
	}
	llvm::Value *Expr_eq::compile(){
		return nullptr;
	}
	llvm::Value *Expr_neq::compile(){
		return nullptr;
	}
	llvm::Value *Expr_lt::compile(){
		return nullptr;
	}
	llvm::Value *Expr_lte::compile(){
		return nullptr;
	}
	llvm::Value *Expr_gt::compile(){
		return nullptr;
	}
	llvm::Value *Expr_gte::compile(){
		return nullptr;
	}
	llvm::Value *Expr_or::compile(){
		return nullptr;
	}
	llvm::Value *Expr_and::compile(){
		return nullptr;
	}
	llvm::Value *Expr_Expr::compile(){
		return nullptr;
	}

}
}
