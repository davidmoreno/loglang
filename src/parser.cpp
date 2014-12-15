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

#include "parser.hpp"
#include "tokenizer.hpp"
#include "ast.hpp"
#include "ast_all.hpp"
#include "program.hpp"

using namespace loglang;

static AST parse_expression(Tokenizer &tokenizer, Token::type_t end, Token::type_t end2=Token::INVALID);


AST loglang::parse_program(const std::string sourcecode){
	Tokenizer tokenizer(sourcecode);
	return parse_expression(tokenizer, Token::_EOF);
}

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


static AST parse_edge_if(Tokenizer &tokenizer, Token::type_t end, Token::type_t end2){
	AST cond=parse_expression(tokenizer, Token::THEN);
	AST if_true=parse_expression(tokenizer, Token::ELSE);
	AST if_false=parse_expression(tokenizer, end, end2);
	return std::make_unique<ast::Edge_if>(std::move(cond), std::move(if_true), std::move(if_false));
}

static AST parse_at(Tokenizer &tokenizer, Token::type_t end, Token::type_t end2){
	AST cond=parse_expression(tokenizer, Token::DO);
	AST _do=parse_expression(tokenizer, end, end2);
	return std::make_unique<ast::At>(std::move(cond), std::move(_do));
}

static AST parse_function_call(Token fname, Tokenizer &tokenizer){
	auto tok=tokenizer.next();
	auto fn=std::make_unique<ast::Function>(fname.token);
	if (tok.type==Token::CLOSE_PAREN)
		return std::move(fn);
	tokenizer.rewind(); // Not good, but necessary
	while(true){
		fn->params.push_back( parse_expression(tokenizer, Token::COMMA, Token::CLOSE_PAREN) );
		tokenizer.rewind();
		tok=tokenizer.next();
		
		if (tok.type==Token::CLOSE_PAREN){
			return std::move(fn);
		}
		else if (tok.type!=Token::COMMA){
			throw parsing_exception(tokenizer.position_to_string() + "; " + std::to_string(tok) + " Invalid function call.");
		}
	}
	throw parsing_exception(tokenizer.position_to_string() + "; Code must never get here");
}

static AST parse_expression(Tokenizer &tokenizer, Token::type_t end, Token::type_t end2){
// 	std::cerr<<" expr "<<tokenizer.position_to_string()<< end <<" " <<end2<<std::endl;
	auto tok=tokenizer.next();
	AST op1;
	if (tok.type==Token::OPEN_PAREN)
		op1=parse_expression(tokenizer, Token::CLOSE_PAREN);
	else if (tok.type==Token::OPEN_CURLY)
		op1=parse_expression(tokenizer, Token::CLOSE_CURLY);
	else if (tok.type==Token::OPEN_BRACKET)
		op1=std::make_unique<ast::Indirect>( parse_expression(tokenizer, Token::CLOSE_BRACKET) );
	else if (tok.type==Token::CLOSE_CURLY && end==Token::CLOSE_CURLY) // Empty {} 0.
		return std::make_unique<ast::Value>(Token("0", Token::NUMBER));
	else if (tok.type==Token::NUMBER || tok.type==Token::STRING)
		op1=std::make_unique<ast::Value>(tok);
	else if (tok.type==Token::VAR){
		if (std::find(std::begin(tok.token), std::end(tok.token), '*')==std::end(tok.token) && std::find(std::begin(tok.token), std::end(tok.token), '?')==std::end(tok.token))
			op1=std::make_unique<ast::Value_var>(tok);
		else
			op1=std::make_unique<ast::Value_glob>(tok);
	}
	else if (tok.type==Token::EDGE_IF)
		return parse_edge_if(tokenizer, end, end2);
	else if (tok.type==Token::AT)
		return parse_at(tokenizer, end, end2);
	else if (tok.type==Token::OP && tok.token=="*"){ // In place conversion, its really a glob arg.
		tok.type=Token::VAR;
		op1=std::make_unique<ast::Value_glob>(tok);
	}
	else{
		std::stringstream s;
		s<<tokenizer.position_to_string();
		s<<"; Got "<<std::to_string(tok)<<" expected expression.";
		throw unexpected_token_type(s.str());
	}
	auto op=tokenizer.next();
// 	std::cerr<<" op "<<op.token<<" "<<tokenizer.position_to_string()<< end <<" " <<end2<<std::endl;
	if (op.type==Token::OPEN_PAREN){ // If function call set as op1, and get again op
		ASTBase *op1p=&*op1;
		auto var=dynamic_cast<ast::Value_var*>(op1p);
		if (var==nullptr){
			throw semantic_exception(tokenizer.position_to_string() + "; lvalue invalid. Only function names are allowed.");
		}
		op1 = parse_function_call(std::move(var->val), tokenizer);
		op=tokenizer.next();
	}
	if (op.type==end || op.type==end2)
		return op1;

	
	AST op2=parse_expression(tokenizer, end, end2);
	
	if (op.type==Token::OP){
		if (op.token=="="){
			ASTBase *op1p=&*op1;
			{
				auto var=dynamic_cast<ast::Value_var*>(op1p);
				if (var)
					return std::make_unique<ast::Equal>(std::move(var->val), std::move(op2));
			}
			{
				auto var2=dynamic_cast<ast::Indirect*>(op1p);
				if (var2)
					return std::make_unique<ast::LVEqual>(std::move(op1), std::move(op2));
			}
			throw semantic_exception(tokenizer.position_to_string() + "; lvalue invalid. Only variables are allowed.");
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
		else if (op.token==";")
			return std::make_unique<ast::Expr_Expr>(std::move(op1), std::move(op2));
		else if (op.token=="and")
			return std::make_unique<ast::Expr_and>(std::move(op1), std::move(op2));
		else if (op.token=="or")
			return std::make_unique<ast::Expr_or>(std::move(op1), std::move(op2));
		else
			throw unexpected_token_type(tokenizer.position_to_string() + "; Cant parse this type of op yet. ("+op.token+")");
	}
	throw parsing_exception(tokenizer.position_to_string() + "; Invalid expression.");
}
