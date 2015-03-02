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
#include "double.hpp"

namespace loglang{
	double_t double_type; // Fast type check (&double_type). Boxing and unboxing.
	
	class double_v : public value_base{
	public:
		double val;
		double_v(int64_t d) : value_base(&double_type), val(d) {}
		std::unique_ptr< value_base > clone() const override;
	};
	
	double_t::double_t() : type_base(type_base::INT){
		name="double";
	}
	
	value double_t::create(double v) const{
		return std::make_unique<double_v>(v);
	}
	double double_t::to_double(const value &v){
		double_v *iv=(double_v*)(&v);
		return iv->val;
	}
	
	std::string double_t::repr(const value &v) const{
		double_v *iv=(double_v*)(&(*v));
		return std::to_string(iv->val);
	}

	
	value double_t::create() const
	{
		return create(0);
	}

	
	namespace ast{
		class double_binary : public ASTBase{
			public:
				AST op1;
				AST op2;
			double_binary(AST _op1, AST _op2) : op1(std::move(_op1)), op2(std::move(_op2)){
				type=static_cast<type_base*>(&double_type);
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
				double i1=double_type.to_double(r1);
				double i2=double_type.to_double(r1);
				return double_type.create( do_op(i1,i2) );
			}
			virtual double do_op(double a, double b) = 0;
			std::string to_string_(const std::string &opname){
				return std::string("<")+opname+" "+op1->to_string()+" "+op2->to_string()+">";
			}
		};
		class double_add : public double_binary{
		public:
			double_add(AST _op1, AST _op2) : double_binary(std::move(_op1), std::move(_op2)){}
			virtual double do_op(double a, double b){
				return a+b;
			}
			virtual std::string to_string(){
				return to_string_("+");
			}
		};
		class double_sub : public double_binary{
		public:
			double_sub(AST _op1, AST _op2) : double_binary(std::move(_op1), std::move(_op2)){}
			virtual double do_op(double a, double b){
				return a-b;
			}
			virtual std::string to_string(){
				return to_string_("-");
			}
		};
		class double_div : public double_binary{
		public:
			double_div(AST _op1, AST _op2) : double_binary(std::move(_op1), std::move(_op2)){}
			virtual double do_op(double a, double b){
				return a / b;
			}
			virtual std::string to_string(){
				return to_string_("/");
			}
		};
	};
};

namespace loglang{
	AST double_t::codegen_f(type_f fid, std::vector< AST > args){
		switch(fid){
			case type_f::ADD:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::double_add>(std::move(args[0]), std::move(args[1]));
				break;
			case type_f::SUB:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::double_sub>(std::move(args[0]), std::move(args[1]));
				break;
			case type_f::DIV:
				if (args.size()!=2)
					throw invalid_argument_count(2);
				return std::make_unique<ast::double_div>(std::move(args[0]), std::move(args[1]));
				break;
			default:
				break;
		}
		throw unknown_function(&double_type, fid);
	}
	
	std::unique_ptr< value_base > double_v::clone() const
	{
		return double_type.create(val);
	}
	
	
}
