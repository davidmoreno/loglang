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
#include "string.hpp"

namespace loglang{
	string_t string_type; // Fast type check (&string_type). Boxing and unboxing.
	
	class string_v : public value_base{
	public:
		std::string val;
		string_v(std::string d) : value_base(&string_type), val(std::move(d)) {}
		std::unique_ptr< value_base > clone() const override;
	};
	
	string_t::string_t() : type_base(type_base::INT){
		name="double";
	}
	
	value string_t::create(std::string v) const{
		return std::make_unique<string_v>(std::move(v));
	}
	
	value string_t::create() const
	{
		return create("");
	}
	
	std::string string_t::to_string(const value &v){
		string_v *iv=(string_v*)(&v);
		return iv->val;
	}
	
	namespace ast{
		class string_binary : public ASTBase{
			public:
				AST op1;
				AST op2;
			string_binary(AST _op1, AST _op2) : op1(std::move(_op1)), op2(std::move(_op2)){
				type=static_cast<type_base*>(&string_type);
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
				auto i1=string_type.to_string(r1);
				auto i2=string_type.to_string(r1);
				return string_type.create( do_op(i1,i2) );
			}
			virtual std::string do_op(std::string a, std::string b) = 0;
			std::string to_string_(const std::string &opname){
				return std::string("<")+opname+" "+op1->to_string()+" "+op2->to_string()+">";
			}
		};
		class string_add : public string_binary{
		public:
			string_add(AST _op1, AST _op2) : string_binary(std::move(_op1), std::move(_op2)){}
			virtual std::string do_op(std::string a, std::string b){
				return a+b;
			}
			virtual std::string to_string(){
				return to_string_("+");
			}
		};
	};
};

namespace loglang{
	AST string_t::codegen_f(type_f fid, std::vector< AST > args){
		switch(fid){
			case type_f::ADD:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::string_add>(std::move(args[0]), std::move(args[1]));
				break;
			default:
				break;
		}
		throw unknown_function(&string_type, fid);
	}
	
	std::unique_ptr< value_base > string_v::clone() const
	{
		return string_type.create(val);
	}
	
	
}
