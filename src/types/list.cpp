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
#include "list.hpp"

namespace loglang{
	list_t list_type; // Fast type check (&list_type). Boxing and unboxing.
	
	class list_v : public value_base{
	public:
		list_t::unboxed val;
		list_v(list_t::unboxed d) : value_base(&list_type), val(std::move(d)) {}
		std::unique_ptr< value_base > clone() const override;
	};
	
	list_t::list_t() : type_base(type_base::INT){
		name="bool";
	}
	
	value list_t::create(list_t::unboxed v) const{
		return std::make_unique<list_v>(std::move(v));
	}
	value list_t::create() const
	{
		return create({});
	}
	
	std::vector<value> list_t::unbox(const value &v) const{
// 		list_v *iv=(list_v*)(&v);
// 		return iv->val;
		list_t::unboxed ret;
		return ret;
	}
	
	namespace ast{
		class list_binary : public ASTBase{
			public:
				AST op1;
				AST op2;
			list_binary(AST _op1, AST _op2) : op1(std::move(_op1)), op2(std::move(_op2)){
				type=static_cast<type_base*>(&list_type); // static_cast not possible as dont know really list_type yet.
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
				auto i1=list_type.unbox(r1);
				auto i2=list_type.unbox(r1);
				return list_type.create( std::move( do_op(i1,i2) ) );
			}
			virtual list_t::unboxed do_op(const list_t::unboxed &a, const list_t::unboxed &b) = 0;
			std::string to_string_(const std::string &opname){
				return std::string("<")+opname+" "+op1->to_string()+" "+op2->to_string()+">";
			}
		};
// 		class list_and : public list_binary{
// 		public:
// 			list_and(AST _op1, AST _op2) : list_binary(std::move(_op1), std::move(_op2)){}
// 			virtual bool do_op(bool a, bool b){
// 				return a && b;
// 			}
// 			virtual std::string to_string(){
// 				return to_string_("+");
// 			}
// 		};
// 		class list_or : public list_binary{
// 		public:
// 			list_or(AST _op1, AST _op2) : list_binary(std::move(_op1), std::move(_op2)){}
// 			virtual bool do_op(bool a, bool b){
// 				return a || b;
// 			}
// 			virtual std::string to_string(){
// 				return to_string_("-");
// 			}
// 		};
	};
};

namespace loglang{
	AST list_t::codegen_f(type_f fid, std::vector< AST > args){
		switch(fid){
// 			case type_f::AND:
// 				if (args.size()!=2)
// 					throw invalid_argument_count(2);
// 				return std::make_unique<ast::list_and>(std::move(args[0]), std::move(args[1]));
// 				break;
// 			case type_f::OR:
// 				if (args.size()!=2)
// 					throw invalid_argument_count(2);
// 				return std::make_unique<ast::list_or>(std::move(args[0]), std::move(args[1]));
// 				break;
			default:
				break;
		}
		throw unknown_function(&list_type, fid);
	}
	
	std::unique_ptr< value_base > list_v::clone() const
	{
		return list_type.create(); // val
	}
}
