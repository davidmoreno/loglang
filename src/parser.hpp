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

#include "ast.hpp"
#include "tokenizer.hpp"

namespace loglang{
	class parsing_exception : public std::exception{
		std::string _str;
	public:
		parsing_exception(std::string s) : _str(std::move(s)) {};
		const char *what() const throw(){ return _str.c_str(); }
	};
	class unexpected_token_type : public parsing_exception{
	public:
		unexpected_token_type(std::string s) : parsing_exception(s){};
		unexpected_token_type(const Token &t) : parsing_exception(std::string("Unexpected token type: ")+std::to_string(t)){};
	};
	class unexpected_token : public parsing_exception{
	public:
		unexpected_token(std::string s) : parsing_exception(s){};
		unexpected_token(const Token &t) : parsing_exception(std::string("Unexpected token: ")+std::to_string(t)){};
	};
	class semantic_exception  :public parsing_exception{
	public:
		semantic_exception(std::string s) : parsing_exception(s){};
	};
	
	
	AST parse_program(const std::string &sourcecode);
}
