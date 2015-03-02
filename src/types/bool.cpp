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
#include "bool.hpp"

namespace loglang{
	bool_t bool_type; // Fast type check (&bool_type). Boxing and unboxing.
	
	class bool_v : public value_base{
	public:
		int64_t val;
		bool_v(int64_t d) : value_base(&bool_type), val(d) {}
		std::unique_ptr< value_base > clone() const override;
	};
	
	bool_t::bool_t() : type_base(type_base::INT){
		name="bool";
	}
	
	value bool_t::create(bool v) const{
		return std::make_unique<bool_v>(v);
	}
	value bool_t::create() const
	{
		return create(false);
	}
	
	bool bool_t::to_bool(const value &v) const{
		bool_v *iv=(bool_v*)(&v);
		return iv->val;
	}
	
	namespace ast{
		class bool_binary : public ASTBase{
			public:
				AST op1;
				AST op2;
			bool_binary(AST _op1, AST _op2) : op1(std::move(_op1)), op2(std::move(_op2)){
				type=static_cast<type_base*>(&bool_type); // static_cast not possible as dont know really bool_type yet.
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
				bool i1=bool_type.to_bool(r1);
				bool i2=bool_type.to_bool(r1);
				return bool_type.create( do_op(i1,i2) );
			}
			virtual bool do_op(bool a, bool b) = 0;
			std::string to_string_(const std::string &opname){
				return std::string("<")+opname+" "+op1->to_string()+" "+op2->to_string()+">";
			}
		};
		class bool_and : public bool_binary{
		public:
			bool_and(AST _op1, AST _op2) : bool_binary(std::move(_op1), std::move(_op2)){}
			virtual bool do_op(bool a, bool b){
				return a && b;
			}
			virtual std::string to_string(){
				return to_string_("+");
			}
		};
		class bool_or : public bool_binary{
		public:
			bool_or(AST _op1, AST _op2) : bool_binary(std::move(_op1), std::move(_op2)){}
			virtual bool do_op(bool a, bool b){
				return a || b;
			}
			virtual std::string to_string(){
				return to_string_("-");
			}
		};
	};
};

namespace loglang{
	AST bool_t::codegen_f(type_f fid, std::vector< AST > args){
		switch(fid){
			case type_f::AND:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::bool_and>(std::move(args[0]), std::move(args[1]));
				break;
			case type_f::OR:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::bool_or>(std::move(args[0]), std::move(args[1]));
				break;
			default:
				break;
		}
		throw unknown_function(&bool_type, fid);
	}
	
	std::unique_ptr< value_base > bool_v::clone() const
	{
		return bool_type.create(val);
	}
}
