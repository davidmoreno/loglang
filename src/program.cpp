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
#include <sstream>

#include "program.hpp"
#include "tokenizer.hpp"
#include "ast.hpp"

using namespace loglang;

// static void expect_token(const Token &got, const Token &expect, Tokenizer &tokenizer){
// 	if (got!=expect){
// 		std::stringstream s;
// 		s<<tokenizer.position_to_string();
// 		s<<"Got "<<std::to_string(got)<<" expected token "<<std::to_string(expect);
// 		throw unexpected_token(s.str());
// 	}
// }
// static void expect_token_type(const Token &got, const Token::type_t expect, Tokenizer &tokenizer){
// 	if (got.type!=expect){
// 		std::stringstream s;
// 		s<<tokenizer.position_to_string();
// 		s<<"Got "<<std::to_string(got)<<" expected token type "<<std::to_string(expect);
// 		throw unexpected_token_type(s.str());
// 	}
// }


static AST parse_expression(Tokenizer &tokenizer, Token::type_t end);

static AST parse_edge_if(Tokenizer &tokenizer, Token::type_t end){
	AST cond=parse_expression(tokenizer, Token::THEN);
	AST if_true=parse_expression(tokenizer, Token::ELSE);
	AST if_false=parse_expression(tokenizer, end);
	return std::make_unique<ast::Edge_if>(std::move(cond), std::move(if_true), std::move(if_false));
}

static AST parse_expression(Tokenizer &tokenizer, Token::type_t end){
	auto tok=tokenizer.next();
	AST op1;
	if (tok.type==Token::OPEN_PAREN)
		op1=parse_expression(tokenizer, Token::CLOSE_PAREN);
	else if (tok.type==Token::NUMBER || tok.type==Token::STRING)
		op1=std::make_unique<ast::Value>(tok);
	else if (tok.type==Token::VAR)
		op1=std::make_unique<ast::Value_var>(tok);
	else if (tok.type==Token::EDGE_IF)
		return parse_edge_if(tokenizer, end);
	else{
		std::stringstream s;
		s<<tokenizer.position_to_string();
		s<<"; Got "<<std::to_string(tok)<<" expected expression";
		throw unexpected_token_type(s.str());
	}
	auto op=tokenizer.next();
	if (op.type==end)
		return op1;
	AST op2=parse_expression(tokenizer, end);
	
	if (op.type==Token::OP){
		if (op.token=="="){
			ASTBase *op1p=&*op1;
			auto var=dynamic_cast<ast::Value_var*>(op1p);
			if (var==nullptr){
				throw semantic_exception(tokenizer.position_to_string() + "; lvalue invalid. Only varaibles are allowed.");
			}
			return std::make_unique<ast::Equal>(std::move(var->val), std::move(op2));
		}
		if (op.token=="*")
			return std::make_unique<ast::Expr_mul>(std::move(op1), std::move(op2));
		else if (op.token=="/")
			return std::make_unique<ast::Expr_div>(std::move(op1), std::move(op2));
		else if (op.token=="<")
			return std::make_unique<ast::Expr_lt>(std::move(op1), std::move(op2));
		else if (op.token==">")
			return std::make_unique<ast::Expr_gt>(std::move(op1), std::move(op2));
		else if (op.token==">=")
			return std::make_unique<ast::Expr_gte>(std::move(op1), std::move(op2));
		else if (op.token=="<=")
			return std::make_unique<ast::Expr_lte>(std::move(op1), std::move(op2));
		else if (op.token=="==")
			return std::make_unique<ast::Expr_eq>(std::move(op1), std::move(op2));
		else if (op.token=="!=")
			return std::make_unique<ast::Expr_neq>(std::move(op1), std::move(op2));
		else if (op.token=="+")
			return std::make_unique<ast::Expr_add>(std::move(op1), std::move(op2));
		else if (op.token=="-")
			return std::make_unique<ast::Expr_sub>(std::move(op1), std::move(op2));
		else
			throw unexpected_token_type(tokenizer.position_to_string() + "; Cant parse this type of op yet. ("+op.token+")");
	}
	throw parsing_exception(tokenizer.position_to_string() + "; Invalid expression.");
}


Program::Program(std::string _sourcecode) : sourcecode(std::move(_sourcecode))
{
	Tokenizer tokenizer(sourcecode);

	ast=parse_expression(tokenizer, Token::_EOF);
	
	_dependencies=ast->dependencies();
// 	std::cerr<<"deps "<<std::to_string(_dependencies)<<std::endl;
// 	std::cerr<<"Compile: "<<_sourcecode<<std::endl;
}

void Program::run(LogParser& context)
{
// 	std::cerr<<"Run"<<std::endl;
	if (ast){
		auto output=ast->eval(context);
// 		context.output(output);
	}
}