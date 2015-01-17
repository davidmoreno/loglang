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

#include <algorithm>
#include "tokenizer.hpp"

#include "utils.hpp"
#include "ast.hpp"
#include "context.hpp"

namespace loglang{
	namespace ast{
		class Equal : public ASTBase{
			std::string var;
			AST op2;
		public:
			Equal(std::string var, AST op2) : var(var), op2(std::move(op2)){}
			value eval(Context &context){
				auto op2_res=op2->eval(context);
				context.get_value(var).set(op2_res->clone(), context);
				return op2_res;
			}
			std::set< std::string > dependencies(){
				return op2->dependencies();
			}
			std::string to_string(){
				return "<Equal "+var+" "+op2->to_string()+">";
			};
		};
		class Value : public ASTBase{
		};
		class Value_const : public Value{
		public:
			value val;
			Value_const(Token t){
				switch(t.type){
					case Token::NUMBER:
						if (std::find(std::begin(t.token),std::end(t.token),'.')==std::end(t.token))
							val=std::move(to_value(int64_t(to_number(t.token))));
						else
							val=std::move(to_value(to_number(t.token)));
					break;
					case Token::STRING:
						val=std::move(to_value(t.token));
					break;
					default:
						throw parsing_exception("Unknown value type: "+t.token);
				}
			}
			value eval(Context &context){
				return val->clone();
			}
			
			std::set< std::string > dependencies(){
				return {};
			}
			
			std::string to_string(){
				return "<Value_const "+std::to_string(val)+">";
			};
		};
		class Value_var : public Value{
		public:
			std::string var;
			Value_var(Token _val) : var(_val.token) {}
			value eval(Context &context){
				auto &v=context.get_value(var).get();
				if (!v)
					throw std::runtime_error(std::string("Value <")+var+"> undefined. Cant use yet.");
				return v->clone();
			}
			std::set<std::string> dependencies(){
				return { var };
			}
			std::string to_string(){
				return "<Value_val "+var+">";
			};
		};
		class Value_glob : public Value{
		public:
			std::string var;
			Value_glob(Token _val) : var(_val.token) {}
			value eval(Context &context){
				return context.get_glob_values( var );
			}
			std::set<std::string> dependencies(){
				return { var };
			}
			std::string to_string(){
				return "<ValueGlob "+var+">";
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
		
		/*
		class Expr_mul : public Expr{
		public:
			Expr_mul(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				if (r1->type_name==r2->type_name && r1->type_name=="int")
					return to_value( op1->eval(context)->to_int() * op2->eval(context)->to_int() );
				else
					return to_value( op1->eval(context)->to_double() * op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_mul");
			}
		};
		class Expr_div : public Expr{
		public:
			Expr_div(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				return to_value( r1->to_double() / r2->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_div");
			}
		};
		class Expr_lt : public Expr{
		public:
			Expr_lt(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				return to_value( op1->eval(context)->to_double() < op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_lt");
			}
		};
		class Expr_lte : public Expr{
		public:
			Expr_lte(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				return to_value( op1->eval(context)->to_double() <= op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_lte");
			}
		};
		class Expr_gt : public Expr{
		public:
			Expr_gt(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				return to_value( op1->eval(context)->to_double() > op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_gt");
			}
		};
		class Expr_gte : public Expr{
		public:
			Expr_gte(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				return to_value( op1->eval(context)->to_double() >= op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_gte");
			}
		};
		class Expr_eq : public Expr{
		public:
			Expr_eq(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				return to_value( op1->eval(context)->to_double() == op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_eq");
			}
		};
		class Expr_neq : public Expr{
		public:
			Expr_neq(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				return to_value( op1->eval(context)->to_double() != op2->eval(context)->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_neq");
			}
		};
		class Expr_add : public Expr{
		public:
			Expr_add(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				if (r1->type_name==r2->type_name){
					if (r1->type_name=="string")
						return to_value( r1->to_string() + r2->to_string() );
					if (r1->type_name=="int")
						return to_value( r1->to_int() + r2->to_int() );
				}
				return to_value( r1->to_double() + r2->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_add");
			}
		};
		class Expr_sub : public Expr{
		public:
			Expr_sub(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				if (r1->type_name==r2->type_name){
					if (r1->type_name=="int")
						return to_value( r1->to_int() - r2->to_int() );
				}
				return to_value( r1->to_double() - r2->to_double() );
			}
			std::string to_string(){
				return to_string_("Expr_sub");
			}
		};
		class Expr_and : public Expr{
		public:
			Expr_and(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				return to_value( r1->to_bool() && r2->to_bool() );
			}
			std::string to_string(){
				return to_string_("Expr_and");
			}
		};
		class Expr_or : public Expr{
		public:
			Expr_or(AST op1, AST op2) : Expr(std::move(op1), std::move(op2)) {}
			value eval(Context &context){
				auto r1=op1->eval(context);
				auto r2=op2->eval(context);
				return to_value( r1->to_bool() || r2->to_bool() );
			}
			std::string to_string(){
				return to_string_("Expr_and");
			}
		};
		*/
		class Block : public ASTBase{
		public:
			std::vector<AST> stmts;
			Block() {}
			value eval(Context &context){
				value ret;
				for(auto &st:stmts)
					ret=std::move( st->eval(context) ); 
				return ret;
			}
			std::string to_string(){
				std::stringstream ss;
				ss<<"{ ";
				for(auto &st: stmts){
					ss<<st->to_string()<<"; ";
				}
				ss<<"}";
				
				return ss.str(); 
			}
			std::set< std::string > dependencies(){
				std::set< std::string > res;
				for(auto &st:stmts)
					for(auto &s: st->dependencies())
						res.insert(s);
				return  res;
			}
		};
		
		class Edge_if : public Expr{
		public:
			AST cond;
			bool prev_value;
			
			Edge_if(AST _cond, AST if_true, AST if_false) : Expr(std::move(if_true), std::move(if_false)) , cond(std::move(_cond)), prev_value(false) {
			}
			value eval(Context &context){
				/*
				bool current=cond->eval(context)->to_bool();
				if (current!=prev_value){
					prev_value=current;
					if (current)
						return op1->eval(context);
					else
						return op2->eval(context);
				}
				*/
				return to_value(false);
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
			value prev_value;
			
			At(AST _cond, AST _do) : Expr(std::move(_cond), std::move(_do)) {
			}
			value eval(Context &context){
				value current=op1->eval(context);
				if (current!=prev_value){
					prev_value=std::move(current);
					return op2->eval(context);
				}
				return to_value("");
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
			value eval(Context& context){
				std::vector<value> args;
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
};