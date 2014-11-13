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
#include <vector>
#include <memory>

namespace loglang{
	class unexpected_token_type : public std::exception{
		std::string _str;
	public:
		unexpected_token_type(std::string s) : _str(std::move(s)) {};
		const char *what() const throw(){ return _str.c_str(); }
	};
	class unexpected_token : public std::exception{
		std::string _str;
	public:
		unexpected_token(std::string str) : _str(std::move(str)) {};
		const char *what() const throw(){ return _str.c_str(); }
	};
	
	class LogParser;
	class ASTBase;
	
	class Program{
		std::string sourcecode;
		std::vector<std::string> _dependencies;
		std::shared_ptr<ASTBase> ast;
		
	public:
		Program(std::string sourcecode);
		const std::vector<std::string> &dependencies() const { return _dependencies; }
		
		void run(LogParser &context);
	};
}
