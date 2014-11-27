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

#pragma once

#include <string>
#include <set>
#include <algorithm>

#include "logparser.hpp"
#include "tokenizer.hpp"
#include "utils.hpp"
#include "value.hpp"

namespace loglang{
	class ASTBase{
	public:
		virtual any eval(LogParser &context) = 0;
		virtual std::string to_string() = 0;
		virtual std::set<std::string> dependencies() = 0;
	};
	
	using AST=std::unique_ptr<ASTBase>;
	
	namespace ast{
		class Equal : public ASTBase{
			Token op1;
			AST op2;
		public:
			Equal(Token op1, AST op2) : op1(op1), op2(std::move(op2)){}
			any eval(LogParser &context){
				auto op2_res=op2->eval(context);
				context.get_value(op1.token).set(op2_res->clone(), context);
				return op2_res;
			}
			std::set< std::string > dependencies(){
				return op2->dependencies();
			}
			std::string to_string(){
				return "<Equal "+op1.token+" "+op2->to_string()+">";
			};
		};
		class Value : public ASTBase{
		public:
			Token val;
			Value(Token _val) : val(_val) {}
			any eval(LogParser &context){
				switch(val.type){
					case Token::NUMBER:
						return to_any(to_number(val.token));
					case Token::STRING:
						return to_any(val.token);
					default:
						throw parsing_exception("Unknown value type: "+val.token);
				}
			}
			
			std::set< std::string > dependencies(){
				return {};
			}
			
			std::string to_string(){
				return "<Value "+val.token+">";
			};
		};
		class Value_var : public Value{
		public:
			Value_var(Token _val) : Value(_val) {}
			any eval(LogParser &context){
				auto &v=context.get_value(val.token).get();
				if (!v)
					throw std::runtime_error(std::string("Value <")+val.token+"> undefined. Cant use yet.");
				return v->clone();
			}
			std::set<std::string> dependencies(){
				return { val.token };
			}
			std::string to_string(){
				return "<ValueVal "+val.token+">";
			};
		};
		class Value_glob : public Value{
		public:
			Value_glob(Token _val) : Value(_val) {}
			any eval(LogParser &context){
				return context.get_glob_values( val.token );
			}
			std::set<std::string> dependencies(){
				return { val.token };
			}
			std::string to_string(){
				return "<ValueGlob "+val.token+">";
			};
		};
		
		class Expr : public ASTBase{
		public:
			AST op1;
			AST op2;
			Expr(AST op1, AST op2) : op1(std::move(op1)), op2(std::move(op2)){}
			std::set< std::string > dependencies(){
				auto res=op1->dependencies();
				for(auto &s: op2->dependencies())
					res.insert(s);
				return  res;
			}
			std::string to_string_(const std::string &op){
				return "<"+op+" "+op1->to_string()+" "+op2->to_string()+">";
			};
		};
		
		class Expr_mul : public Expr{
		public:
			Expr_mul(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() * op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_mul");
			}
		};
		class Expr_div : public Expr{
		public:
			Expr_div(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				return to_any( r1->to_double() / r2->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_div");
			}
		};
		class Expr_lt : public Expr{
		public:
			Expr_lt(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() < op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_lt");
			}
		};
		class Expr_lte : public Expr{
		public:
			Expr_lte(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() <= op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_lte");
			}
		};
		class Expr_gt : public Expr{
		public:
			Expr_gt(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() > op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_gt");
			}
		};
		class Expr_gte : public Expr{
		public:
			Expr_gte(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() > op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_gte");
			}
		};
		class Expr_eq : public Expr{
		public:
			Expr_eq(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() == op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_eq");
			}
		};
		class Expr_neq : public Expr{
		public:
			Expr_neq(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() != op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_neq");
			}
		};
		class Expr_add : public Expr{
		public:
			Expr_add(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				return to_any( op1->eval(context)->to_double() + op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_add");
			}
		};
		class Expr_sub : public Expr{
		public:
			Expr_sub(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			any eval(LogParser &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				return to_any( r1->to_double() - r2->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_sub");
			}
		};
		
		class Edge_if : public Expr{
		public:
			AST cond;
			bool prev_value;
			
			Edge_if(AST _cond, AST if_true, AST if_false) : Expr(std::move(if_true), std::move(if_false)) , cond(std::move(_cond)), prev_value(false) {
			}
			any eval(LogParser &context){
				bool current=cond->eval(context)->to_int();
				if (current!=prev_value){
					prev_value=current;
					if (current)
						return op1->eval(context);
					else
						return op2->eval(context);
				}
				return to_any(false);
			}
			std::set< std::string > dependencies(){
				return cond->dependencies();
			}
			std::string to_string(){
				return "<Edge_if "+cond->to_string()+" "+op1->to_string()+""+op2->to_string()+">";
			}
		};
		class At : public Expr{
		public:
			any prev_value;
			
			At(AST _cond, AST _do) : Expr(std::move(_cond), std::move(_do)) {
			}
			any eval(LogParser &context){
				any current=op1->eval(context);
				if (current!=prev_value){
					prev_value=std::move(current);
					return op2->eval(context);
				}
				return to_any("");
			}
			std::set< std::string > dependencies(){
				return op1->dependencies();
			}
			std::string to_string(){
				return to_string_("At");
			}
		};
		class Function : public ASTBase{
		public:
			Function(std::string fn) : fnname(std::move(fn)){}
			std::string fnname;
			std::vector<AST> params;
			any eval(LogParser& context){
				std::vector<any> args;
				for(auto &ev: params){
					args.push_back(ev->eval(context));
				}
				
				return context.fn(fnname, args);
			}
			std::set< std::string > dependencies(){
				auto res=std::set< std::string >();
				for(auto &el: params){
					for(auto &s: el->dependencies())
						res.insert(s);
				}
				return  res;
			}
			std::string to_string(){
				std::string params_str;
				bool first=true;
				for(auto &a: params){
					if (!first)
						params_str+=", ";
					params_str+=a->to_string();
				}
				
				return "<Function "+fnname+" {"+params_str+"}>";
			}
		};
	}
}
