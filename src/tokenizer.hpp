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
#include <sstream>

namespace loglang{
	class unexpected_char : public std::exception{
		std::string str;
	public:
		unexpected_char(const std::string &error_context, char _c){ str=error_context+std::string("Unexpected char "); str.append(std::string(&_c,1)); }
		const char *what() const throw(){
			return str.c_str();
		}
	};
	
	
	class Token{
	public:
		enum type_t{ // Numebrs dont really matter but for debugging its nice to have a list with token_t and number
			_EOF=0,
			VAR=1,
			OP=2,
			STRING=3,
			NUMBER=4,
			OPEN_PAREN=5,
			CLOSE_PAREN=6,
			IF=7,
			EDGE_IF=8,
			THEN=9,
			ELSE=10,
			AT=11,
			DO=12,
			COMMA=13,
			
			INVALID=255
		};
		
		
		std::string token;
		Token::type_t type;
		
		Token(std::string token, Token::type_t type) : token(token), type(type){}
		Token(char c, Token::type_t type) : token(&c,1), type(type){}
		Token() : type(_EOF){};
		
		operator bool(){ // True means that its not EOF
			return type!=_EOF;
		}
		
		bool operator==(const Token &other) const{
			return type==other.type && token==other.token;
		}
		bool operator!=(const Token&other) const{
			return !((*this)==other);
		}
	};
	
	class Tokenizer{
		std::string data;
		std::string::iterator pos;
		Token last_token;
		bool _rewind=false;
	private:
		Token real_next();
	public:
		Tokenizer(std::string str) : data(std::move(str)), pos(std::begin(data)){}
		
		Token next();
		void rewind();
		
		std::string position_to_string();
	};
}

namespace std{
	std::string to_string(const loglang::Token &t);
}
