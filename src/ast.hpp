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

#include <string>

#include "logparser.hpp"
#include "tokenizer.hpp"

namespace loglang{
	double to_number(const std::string &str){
		return std::atof(str.c_str());
	}
	
	class ASTBase{
	public:
		virtual std::string eval(LogParser &context) = 0;
	};
	
	using AST=std::unique_ptr<ASTBase>;
	
	namespace ast{
		class Equal : public ASTBase{
			Token op1;
			AST op2;
		public:
			Equal(Token op1, AST op2) : op1(op1), op2(std::move(op2)){}
			std::string eval(LogParser &context){
				auto op2_res=op2->eval(context);
				context.get_value(op1.token).set(op2_res, context);
				return op2_res;
			}
		};
		class Value : public ASTBase{
		public:
			Token val;
			Value(Token _val) : val(_val) {}
			std::string eval(LogParser &context){
				return val.token;
			}
		};
		class Value_var : public Value{
		public:
			Value_var(Token _val) : Value(_val) {}
			std::string eval(LogParser &context){
				return context.get_value(val.token).get();
			}
		};
		
		class Expr : public ASTBase{
		public:
			AST op1;
			AST op2;
			Expr(AST op1, AST op2) : op1(std::move(op1)), op2(std::move(op2)){}
		};
		
		class Expr_mul : public Expr{
		public:
			Expr_mul(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			std::string eval(LogParser &context){
				return std::to_string( to_number(op1->eval(context)) * to_number(op2->eval(context)) );
			}
		};
		class Expr_div : public Expr{
		public:
			Expr_div(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			std::string eval(LogParser &context){
				return std::to_string( to_number(op1->eval(context)) / to_number(op2->eval(context)) );
			}
		};
	}
}
