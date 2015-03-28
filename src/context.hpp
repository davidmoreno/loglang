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
#include <functional>
#include <unordered_map>

#include "symbol.hpp"
// #include "program.hpp"

namespace loglang{
	class Program;
	
	class Context : public std::enable_shared_from_this<Context>{
		std::function<void (const std::string &output)> _output;
		std::unordered_map<std::string, Symbol> symboltable;
		std::unordered_map<std::string, std::shared_ptr<Program>> glob_dependencies_programs;
		std::unordered_map<std::string, std::shared_ptr<Program>> programs;
		std::unordered_map<std::string, std::function<any (Context &, const std::vector<any> &)>> functions;
	public:
		Context();
		void feed_secure(std::string data);
		void feed(std::string data);
		void set_output(std::function<void (const std::string &output)> &&);
		void output(const std::string &str){ _output(str); }
		void output(const std::string &str, const std::string &str2);
		Symbol &get_value(const std::string &key);
		
		/// Returns the resolved glob values. 
		any get_glob_values(const std::string &glob);
		std::vector<Symbol*> symboltable_filter(const std::string &glob);
		
		void register_function(std::string fnname, std::function<any (Context &, const std::vector<any> &)> f);
		any fn(const std::string &fname, const std::vector<any> &args);
		
		void debug_values();
	};
};
