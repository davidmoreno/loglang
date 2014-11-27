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
#include <set>
#include <stdexcept>

#include "tokenizer.hpp"

using namespace loglang;

static std::string ops="*/+-%=<>!";
static std::string ops2letter_1="><=!";
// static std::string ops2letter_2="="; // Just one option, better simple =

static std::set<std::string> extraops{"<=",">=","and","or","=="};
static std::string number="0123456789.";
static std::string var_extra_letters="_-%.*?";

Token Tokenizer::real_next()
{
	auto data_end=std::end(data);
	while (std::isspace(*pos) && pos<data_end) ++pos; // Skip spaces
	if (pos>=data_end){
		return Token();
	}
	
	Token::type_t type=Token::_EOF;
	if (*pos=='"') // For multichar tokens, set type.
		type=Token::STRING;
	else if (std::isalpha(*pos) || *pos=='_')
		type=Token::VAR;
	else if (*pos=='%')
		return Token(*pos++,Token::VAR);
	else if (std::isdigit(*pos))
		type=Token::NUMBER;
	else if (*pos=='(') // For unichar tokens, return.
		return Token(*pos++, Token::OPEN_PAREN);
	else if (*pos==',') 
		return Token(*pos++, Token::COMMA);
	else if (*pos==')')
		return Token(*pos++, Token::CLOSE_PAREN);
	else if (std::find(std::begin(ops), std::end(ops), *pos)!=std::end(ops)){
		auto c=*pos++;
		auto tok=Token(c, Token::OP);
		
		if (std::find(std::begin(ops), std::end(ops), c)!=std::end(ops2letter_1)){ // 2 letter op (>=, == ...), first part
			if (*pos=='='){ // 2 letter op (>=, == ...), second part
				tok.token+=*pos;
				++pos;
			}
		}
		return tok;
	}
	else
		throw unexpected_char(position_to_string(), *pos);
	auto start=pos;
	++pos;
	
	std::string str;
	// For multi char tokens, continue parsing.
	if (type==Token::STRING){
		while (*pos!='"' && pos<data_end) ++pos;
		str=std::string(start+1, pos);
		++pos;
	}
	else if (type==Token::NUMBER){
		while (std::find(std::begin(number), std::end(number), *pos)!=std::end(number) && pos<data_end) ++pos;
		str=std::string(start, pos);
	}
	else if (type==Token::VAR){
		while ((std::isalnum(*pos) || std::find(std::begin(var_extra_letters), std::end(var_extra_letters), *pos)!=std::end(var_extra_letters)) && pos<data_end) ++pos; // Skip spaces
		str=std::string(start, pos);
		if (str=="if")
			type=Token::IF;
		if (str=="then")
			type=Token::THEN;
		if (str=="else")
			type=Token::ELSE;
		if (str=="edge_if")
			type=Token::EDGE_IF;
		if (str=="at")
			type=Token::AT;
		if (str=="do")
			type=Token::DO;
		if (std::find(std::begin(extraops), std::end(extraops), str)!=std::end(extraops))
			type=Token::OP;
	}
	
// 	std::cerr<<"got Token "<<str<<std::endl;
	
	return Token(std::move(str), type);
}

Token Tokenizer::next()
{
	if (_rewind){
		_rewind=false;
		return last_token;
	}
	last_token = real_next();
	return last_token;
}

void Tokenizer::rewind(){
	if (_rewind)
		throw std::runtime_error("Tokenizer, trying to rewind twice.");
	_rewind=true;
}


std::string Tokenizer::position_to_string(){
	return std::string(std::begin(data), pos-(last_token.token.length()+1)) + "\033[01;31m"+last_token.token+"\033[0m" + std::string(pos, std::end(data));
}

std::string std::to_string(const Token& t){
	std::stringstream s;
	s<<"<Token '"<<t.token<<"', type "<<t.type<<">";
	return s.str();
}

