/*
 * Copyright 2014-2015 David Moreno
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

#include <memory>
#include <string>
#include <unordered_map>

#include "ast.hpp"

namespace loglang{
	class unknown_function : public std::exception{	};
	class invalid_argument_count : public std::exception{
	public:
		std::string txt;
		invalid_argument_count(int n){
			txt=std::string("Expected ")+std::to_string(n)+std::string(" arguments");
		};
		const char* what() const throw(){
			return txt.c_str();
		}
	};
	class invalid_function_argument_type : public std::exception{
	public:
		std::string txt;
		invalid_function_argument_type(int argc, const std::string &expected){
			txt=std::string("Expected ")+ expected + " at argument "+std::to_string(argc);
		};
		const char* what() const throw(){
			return txt.c_str();
		}
	};
	
	class type_base;
	
	using type=type_base *;
	
	enum class type_f{
		NONE=0,
		LT,
		GT,
		LTE,
		GTE,
		EQ,
		NEQ,
		ADD,
		SUB,
		DIV,
		MUL,
		MOD,
		AND,
		OR,
		IN,
	};
	
	class type_base{
	public:
		enum type_id{
			INT,
			STRING,
			BOOL,
			DOUBLE,
			VOID,
		};
		
		
		static std::unordered_map<type_id, type,  std::hash<int>> types;
		static type get(type_base::type_id tid);
	public:
		std::string name;
		type_base(type_base::type_id t);
		virtual ~type_base(){}
		virtual AST codegen_f(type_f fid, std::vector<AST> args) = 0;
// 		virtual AST codegen_f(type_f fid, std::initializer_list<AST &&> &&args);
// 		virtual AST codegen_f(type_f fid, std::initializer_list<AST &&> &&args);
	};
	
	
	value to_value(int64_t v);
	value to_value(double v);
	value to_value(std::string v);
	value to_value(bool v);
	value to_value(std::vector<value> v);
}
