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

static void expect_token(const Token &got, const Token &expect){
	if (got!=expect){
		std::stringstream s;
		s<<"Got "<<std::to_string(got)<<" expected "<<std::to_string(expect);
		throw unexpected_token(s.str());
	}
}
static void expect_token_type(const Token &got, const Token::type_t expect){
	if (got.type!=expect){
		std::stringstream s;
		s<<"Got "<<std::to_string(got)<<" expected "<<std::to_string(expect);
		throw unexpected_token_type(s.str());
	}
}

static AST parse_expression(Tokenizer &tokenizer, Token::type_t end=Token::_EOF){
	auto tok=tokenizer.next();
	AST op1;
	if (tok.type==Token::OPEN_PAREN)
		op1=parse_expression(tokenizer, Token::CLOSE_PAREN);
	else if (tok.type==Token::NUMBER || tok.type==Token::STRING)
		op1=std::make_unique<ast::Value>(tok);
	else if (tok.type==Token::VAR)
		op1=std::make_unique<ast::Value_var>(tok);
	else{
		std::stringstream s;
		s<<"Got "<<std::to_string(tok)<<" expected expression";
		throw unexpected_token_type(s.str());
	}
	auto op=tokenizer.next();
	if (op.type==end)
		return op1;
	AST op2=parse_expression(tokenizer, end);
	
	if (op.token=="*")
		return std::make_unique<ast::Expr_mul>(std::move(op1), std::move(op2));
	else if (op.token=="/")
		return std::make_unique<ast::Expr_div>(std::move(op1), std::move(op2));
	else
		throw unexpected_token_type("Cant parse this type of op yet.");
}

Program::Program(std::string _sourcecode) : sourcecode(std::move(_sourcecode))
{
	Tokenizer tokenizer(sourcecode);
	Token tok=tokenizer.next();

	expect_token_type(tok, Token::VAR);
	auto op1=tok;
	tok=tokenizer.next();
	expect_token(tok, Token('=',Token::OP));
	auto op2=parse_expression(tokenizer);
	
	ast=std::make_unique<ast::Equal>(op1, std::move(op2));
	
	_dependencies.push_back("mem.free");
	_dependencies.push_back("mem.total");
	
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
