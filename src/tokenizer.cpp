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
#include <algorithm>
#include "tokenizer.hpp"

using namespace loglang;

static std::string ops="*/+-%=";
static std::string number="0123456789.";
static std::string var_extra_letters="_-%.";

Token Tokenizer::next()
{
	auto data_end=std::end(data);
	while (std::isspace(*pos) && pos<data_end) ++pos; // Skip spaces
	if (pos>=data_end){
		return Token();
	}
	
	Token::type_t type=Token::_EOF;
	if (*pos=='"') // For multichar tokens, set type.
		type=Token::STRING;
	else if (std::isalpha(*pos))
		type=Token::VAR;
	else if (std::isdigit(*pos))
		type=Token::NUMBER;
	else if (*pos=='(') // For unichar tokens, return.
		return Token(*pos++, Token::OPEN_PAREN);
	else if (*pos==')')
		return Token(*pos++, Token::CLOSE_PAREN);
	else if (std::find(std::begin(ops), std::end(ops), *pos)!=std::end(ops))
		return Token(*pos++, Token::OP);
	else
		throw unexpected_char(*pos);
	auto start=pos;
	++pos;
	
	// For multi char tokens, continue parsing.
	if (type==Token::STRING)
		while (*pos!='"' && pos<data_end) ++pos;
	else if (type==Token::NUMBER)
		while (std::find(std::begin(number), std::end(number), *pos)!=std::end(number) && pos<data_end) ++pos;
	else if (type==Token::VAR)
		while ((std::isalnum(*pos) || std::find(std::begin(var_extra_letters), std::end(var_extra_letters), *pos)!=std::end(var_extra_letters)) && pos<data_end) ++pos; // Skip spaces
		
		
	std::string str(start, pos);
// 	std::cerr<<"got Token "<<str<<std::endl;
	
	return Token(std::move(str), type);
}

std::string std::to_string(const Token& t){
	std::stringstream s;
	s<<"<Token '"<<t.token<<"', type "<<t.type<<">";
	return s.str();
}
