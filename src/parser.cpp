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

#include "parser.hpp"
#include "tokenizer.hpp"
#include "ast.hpp"
#include "ast_all.hpp"
#include "program.hpp"

using namespace loglang;
namespace loglang{
	
	class Parser{
		Tokenizer tokenizer;
		
		AST parse_stmt();
		AST parse_expr();
		AST parse_expr2();
		AST parse_expr3();
		AST parse_expr4();
		AST parse_term();
		AST parse_val();
		
		void parse_arg_list(std::unique_ptr<ast::Function> &f);
		
		AST parse_block();
		
		Token &assert_next(Token::type_t t);
	public:
		Parser(const std::string &sourcecode);
		AST parse();
	};
	
	AST parse_program(const std::string &sourcecode){
		Parser parser(sourcecode);
		return parser.parse();
	}
}

Parser::Parser(const std::string& sourcecode) : tokenizer(sourcecode)
{
}

AST Parser::parse()
{
	try{
		AST ret=parse_stmt();
// 		std::cerr<<ret->to_string()<<std::endl;
		return std::move(ret);
	}
	catch(parsing_exception &excp){
		std::cerr<<"Error parsing: "<<excp.what()<<std::endl;
		std::cerr<<tokenizer.position_to_string()<<std::endl;
		throw;
	}
}

Token &Parser::assert_next(Token::type_t expected)
{
	Token &tok=tokenizer.next();
	if (tok.type!=expected){
		print_backtrace();
		std::stringstream s;
		s<<tokenizer.position_to_string();
		s<<"Got "<<std::to_string(tok)<<" expected token type "<<expected;
		throw unexpected_token_type(s.str());
	}
	return tok;
}

AST Parser::parse_stmt()
{
	Token &tok=tokenizer.next();
	if (tok.type==Token::OPEN_CURLY){
		return parse_block();
	}
	tokenizer.rewind();
	return parse_expr();
}

AST Parser::parse_block()
{
	AST retblock=std::make_unique<ast::Block>();
	ast::Block *block=static_cast<ast::Block*>(&*retblock);
	Token &tok=tokenizer.next();
	while (tok.type!=Token::CLOSE_CURLY){
		tokenizer.rewind();
		block->stmts.push_back( std::move(parse_stmt()) );
		assert_next(Token::COLON);
		tok=tokenizer.next();
	}
	return retblock;
}


AST Parser::parse_expr()
{
	AST op1=parse_expr2();
	auto &op=tokenizer.next();
	if (op.type==Token::OP && op.token=="="){
		AST op2=parse_expr();
		ASTBase *op1p=&*op1;
		auto var=dynamic_cast<ast::Value_var*>(op1p);
		if (var==nullptr){
			throw semantic_exception(tokenizer.position_to_string() + "; lvalue invalid. Only variables are allowed.");
		}
		return std::make_unique<ast::Equal>(std::move(var->var), std::move(op2));
	}
	tokenizer.rewind();
	return op1;
}


AST Parser::parse_expr2() // < > <= >= == in and or
{
	AST op1=parse_expr3();
	auto &op=tokenizer.next();
	if (op.type==Token::OP){
		if (op.token=="<"){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_lt>(std::move(op1), std::move(op2));
		}
		if (op.token==">"){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_gt>(std::move(op1), std::move(op2));
		}
		if (op.token=="<="){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_lte>(std::move(op1), std::move(op2));
		}
		if (op.token==">="){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_gte>(std::move(op1), std::move(op2));
		}
		if (op.token=="=="){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_eq>(std::move(op1), std::move(op2));
		}
		if (op.token=="!="){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_neq>(std::move(op1), std::move(op2));
		}
		if (op.token=="and"){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_and>(std::move(op1), std::move(op2));
		}
		if (op.token=="or"){
			AST op2=parse_expr2();
			return std::make_unique<ast::Expr_or>(std::move(op1), std::move(op2));
		}
		if (op.token=="in"){
			AST op2=parse_expr2();
			throw parsing_exception("'in' not yet");
			//return std::make_unique<ast::Expr_gt>(std::move(op1), std::move(op2));
		}
	}
	tokenizer.rewind(); // Ops, not any of those
	return op1;
}

AST Parser::parse_expr3() // + -
{
	AST op1=parse_expr4();
	auto &op=tokenizer.next();
	if (op.type==Token::OP){
		if (op.token=="+"){
			AST op2=parse_expr3();
			return std::make_unique<ast::Expr_add>(std::move(op1), std::move(op2));
		}
		if (op.token=="-"){
			AST op2=parse_expr3();
			return std::make_unique<ast::Expr_sub>(std::move(op1), std::move(op2));
		}
	}
	tokenizer.rewind(); // Ops, not + nor -
	return op1;
}


AST Parser::parse_expr4() // * /
{
	AST op1=parse_term();
	auto &op=tokenizer.next();
	if (op.type==Token::OP){
		if (op.token=="*"){
			AST op2=parse_expr4();
			return std::make_unique<ast::Expr_mul>(std::move(op1), std::move(op2));
		}
		if (op.token=="/"){
			AST op2=parse_expr4();
			return std::make_unique<ast::Expr_div>(std::move(op1), std::move(op2));
		}
	}
	tokenizer.rewind(); // Ops, not / nor *
	return op1;
}



AST Parser::parse_term()
{
	auto &tok=tokenizer.next();
	if (tok.type==Token::IF){
		throw parsing_exception("'if' not yet");
	}
	if (tok.type==Token::EDGE_IF){
		auto cmp=parse_expr();
		assert_next(Token::THEN);
		auto if_true=parse_stmt();
		assert_next(Token::ELSE);
		auto if_false=parse_stmt();
		return std::make_unique<ast::Edge_if>(std::move(cmp), std::move(if_true), std::move(if_false));
	}
	if (tok.type==Token::AT){
		auto at=parse_expr();
		assert_next(Token::DO);
		auto then=parse_stmt();
		return std::make_unique<ast::At>(std::move(at), std::move(then));
	}
	if (tok.type==Token::OPEN_PAREN){
		auto expr=parse_expr();
		assert_next(Token::CLOSE_PAREN);
		return expr;
	}
	if (tok.type==Token::OP && tok.token=="-"){
		auto expr=parse_expr();
		throw parsing_exception("unary op '-' not yet");
		//return std::make_unique<ast::Neg>(std::move(expr));
	}
	tokenizer.rewind();
	return parse_val();
}


AST Parser::parse_val()
{
	Token tok=tokenizer.next();
	if (tok.type==Token::NUMBER || tok.type==Token::STRING){
		return std::make_unique<ast::Value_const>(tok);
	}
	if (tok.token=="*") // All vars glob
		tok.type=Token::VAR; 
	if (tok.type==Token::VAR){
		Token &next=tokenizer.next();
		if (next.type==Token::OPEN_PAREN){
			auto f=std::make_unique<ast::Function>(tok.token);
			parse_arg_list(f);
			assert_next(Token::CLOSE_PAREN);
			return std::move(f);
		}
		tokenizer.rewind();
		if (tok.type==Token::VAR){
			if (std::find(std::begin(tok.token), std::end(tok.token), '*')==std::end(tok.token) && std::find(std::begin(tok.token), std::end(tok.token), '?')==std::end(tok.token))
				return std::make_unique<ast::Value_var>(tok);
			else
				return std::make_unique<ast::Value_glob>(tok);
		}
	}
	throw unexpected_token_type(tok);
}

void Parser::parse_arg_list(std::unique_ptr< ast::Function >& f)
{
	while (true){
		f->params.push_back( std::move(parse_expr()) );
		Token &tok=tokenizer.next();
		if (tok.type==Token::CLOSE_PAREN){
			tokenizer.rewind();
			return;
		}
		tokenizer.rewind();
		assert_next(Token::COMMA);
	}
}
