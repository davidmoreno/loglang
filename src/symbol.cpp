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

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>

#include <algorithm>
#include <iostream>

#include "symbol.hpp"
#include "program.hpp"
#include "context.hpp"
#include "jitprogram.hpp"

using namespace loglang;

Symbol::Symbol(std::string name) : _name(std::move(name)), llvm_val(nullptr), llvm_val_name(nullptr)
{

}


void Symbol::run_at_modify(std::shared_ptr< Program > _at_modify)
{
	at_modify.push_back(_at_modify);
}

void Symbol::remove_program(std::shared_ptr< Program > _at_modify)
{
	at_modify.erase( std::remove(std::begin(at_modify), std::end(at_modify), _at_modify), std::end(at_modify));
}

const loglang::any &Symbol::get() const
{
	return val;
}

void Symbol::set(any new_val, Context &context)
{
	if (val==new_val) // Ignore no changes.
		return; 
	val=std::move(new_val);
// 	context.output(name, value);
	if (_name=="%") // Prevent recursion.
		return;
	context.get_value("%").set(to_any(_name), context);
	for(auto program: at_modify)
		program->run(context);
}

llvm::Value* Symbol::llvm_value()
{
	if (!llvm_val){
		llvm::Module *module=jit::context.module;
		llvm::Constant* const_val=nullptr;
		llvm::Type *type=nullptr;
		if (val->type_name=="double"){
			type=llvm::Type::getDoubleTy(llvm::getGlobalContext());
			const_val=llvm::ConstantFP::get(module->getContext(), llvm::APFloat(val->to_double()));
		}
		else if (val->type_name=="int"){
			type=llvm::Type::getInt64PtrTy(llvm::getGlobalContext());
			const_val=llvm::ConstantInt::get(module->getContext(), llvm::APInt(32,val->to_int()));
		}
		else
			throw std::runtime_error("There is no support for this type yet.");

		module->getOrInsertGlobal(_name, type);
		llvm::GlobalVariable *var=module->getGlobalVariable(_name);
		var->setLinkage(llvm::GlobalValue::CommonLinkage);
// 		var->setAlignment(4);
		var->setInitializer(const_val);

		llvm_val=var;
	}
// 	std::cout<<"Get llvm var for "<<_name<<":"<<val->type_name<<" "<<llvm_val<<" "<<std::to_string(val)<<std::endl;
	return llvm_val;
}

llvm::Value* Symbol::llvm_value_name(){
	if (!llvm_val_name){
		llvm::Module *module=jit::context.module;
		llvm::Type *type=llvm::ArrayType::get(llvm::IntegerType::get(module->getContext(), 8), 2);
		auto vn=std::string("__")+_name+std::string("_name__");
		module->getOrInsertGlobal(vn, type);
		llvm::GlobalVariable *var=module->getGlobalVariable(vn);
		var->setLinkage(llvm::GlobalValue::InternalLinkage);
// 		var->setAlignment(4);
		var->setInitializer(llvm::ConstantDataArray::getString(module->getContext(), _name.c_str(), true));

		llvm_val_name=var;
	}
	return llvm_val_name;
}
