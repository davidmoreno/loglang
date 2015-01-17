/*
 * Copyright 2014-2015 David Moreno
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

#include "../ast.hpp"
#include "../types.hpp"
#include "../value.hpp"
#include "../utils.hpp"
#include "int.hpp"

namespace loglang{
	class int_t;
	int_t int_type; // Fast type check (&int_type). Boxing and unboxing.
	
	class int_v : public value_base{
	public:
		int64_t val;
		int_v(int64_t d) : value_base(&int_type), val(d) {}
		std::unique_ptr< value_base > clone() const override;
	};
	
	int_t::int_t() : type_base(type_base::INT){
		name="int";
	}
	
	value int_t::create(int64_t v){
		return std::make_unique<int_v>(v);
	}
	int64_t int_t::to_int(const value &v){
		int_v *iv=(int_v*)(&v);
		return iv->val;
	}
	
	namespace ast{
		class int_binary : public ASTBase{
			public:
				AST op1;
				AST op2;
			int_binary(AST _op1, AST _op2) : op1(std::move(_op1)), op2(std::move(_op2)){
				type=static_cast<type_base*>(&int_type); // static_cast not possible as dont know really int_type yet.
				if (op1->type!=type)
					throw invalid_function_argument_type(1, type->name);
				if (op2->type!=type)
					throw invalid_function_argument_type(2, type->name);
			}
			virtual std::set< std::string > dependencies(){
				return op1->dependencies() + op2->dependencies();
			}
			virtual std::string to_string(const std::string &op){
				return std::string("<")+op+" "+op1->to_string()+" "+op2->to_string()+">";
			}
			virtual value eval(Context& context){
				value r1=op1->eval(context);
				value r2=op2->eval(context);
				int64_t i1=int_type.to_int(r1);
				int64_t i2=int_type.to_int(r1);
				return int_type.create( do_op(i1,i2) );
			}
			virtual int64_t do_op(int64_t a, int64_t b) = 0;
			std::string to_string_(const std::string &opname){
				return std::string("<")+opname+" "+op1->to_string()+" "+op2->to_string()+">";
			}
		};
		class int_add : public int_binary{
		public:
			int_add(AST _op1, AST _op2) : int_binary(std::move(_op1), std::move(_op2)){}
			virtual int64_t do_op(int64_t a, int64_t b){
				return a+b;
			}
			virtual std::string to_string(){
				return to_string_("+");
			}
		};
		class int_sub : public int_binary{
		public:
			int_sub(AST _op1, AST _op2) : int_binary(std::move(_op1), std::move(_op2)){}
			virtual int64_t do_op(int64_t a, int64_t b){
				return a-b;
			}
			virtual std::string to_string(){
				return to_string_("-");
			}
		};
	};
};

namespace loglang{
	AST int_t::codegen_f(type_f fid, std::vector< AST > args){
		switch(fid){
			case type_f::ADD:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::int_add>(std::move(args[0]), std::move(args[1]));
				break;
			case type_f::SUB:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::int_sub>(std::move(args[0]), std::move(args[1]));
				break;
			default:
				throw unknown_function();
		}
		return nullptr;
	}
	
	std::unique_ptr< value_base > int_v::clone() const
	{
		return int_type.create(val);
	}
	
	
	value to_value(int64_t v){
		return std::make_unique<int_v>(v);
	}
}
