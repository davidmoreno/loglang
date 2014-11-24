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
#include <set>
#include <memory>

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
	};
	class unexpected_token : public parsing_exception{
	public:
		unexpected_token(std::string s) : parsing_exception(s){};
	};
	class semantic_exception  :public parsing_exception{
	public:
		semantic_exception(std::string s) : parsing_exception(s){};
	};
	
	class LogParser;
	class ASTBase;
	
	class Program{
		std::string sourcecode;
		std::set<std::string> _dependencies;
		std::shared_ptr<ASTBase> ast;
		
	public:
		Program(std::string sourcecode);
		const std::set<std::string> &dependencies() const { return _dependencies; }
		
		void run(LogParser &context);
	};
}
