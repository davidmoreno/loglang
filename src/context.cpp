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
#include <sstream>
#include <string>
#include <algorithm>

#include "context.hpp"
#include "program.hpp"
#include "glob.hpp"
#include "utils.hpp"
#include "builtins.hpp"
#include "parser.hpp"

using namespace loglang;

Context::Context()
{
	_output=[](const std::string &str){
		std::cout<<str<<std::endl;
	};
	
	register_builtins(*this);
}

void Context::feed(const std::string& _data)
{
	std::string data=_data;
	if (data.length()==0)
		return;
	{
		auto comment=std::find(std::begin(data), std::end(data), '#');
		if (comment!=std::end(data)){ // Remove comments
			data.erase(comment, std::end(data));
		}
	}
	if (std::isspace(data[0]))
		trim(data);
	if (data.length()==0)
		return;
	
	if (data[0]==':'){ // New program
		auto colonpos=data.find_first_of(' ');
		auto key=data.substr(0, colonpos);
		if (colonpos>=data.length()){ // Remove, no program
			auto prog=programs[key];
			for (auto &dep: prog->dependencies()){
				get_value(dep).remove_program(prog);
				
				auto I=std::begin(glob_dependencies_programs), endI=std::end(glob_dependencies_programs);
				for(; I != endI; ) {
					if (I->second==prog) {
							glob_dependencies_programs.erase(I++);
					} else {
						++I;
					}
				}
			}
			programs.erase(key);
		}
		else{ 
			auto value=data.substr(colonpos+1);
			std::shared_ptr<Program> prog;
			try{
				prog=std::make_shared<Program>(key, std::move(value));
			}
			catch(std::exception &excp){
				std::cerr<<"Error compiling: "<<excp.what()<<std::endl;
				return;
			}
			
			programs[key]=prog;
			for (auto &dep: prog->dependencies()){
				if (std::find(std::begin(dep), std::end(dep), '?')!=std::end(dep) || std::find(std::begin(dep), std::end(dep), '*')!=std::end(dep)){
//  					std::cerr<<"Glob depend "<<dep<<std::endl;
					for (auto &key_value: symboltable){
						if (glob_match(key_value.first, dep)){
// 							std::cerr<<"Match!"<<std::endl;
							get_value(key_value.first).run_at_modify(prog);
						}
					glob_dependencies_programs[dep]=prog;
					}
				}
				else // No glob
					get_value(dep).run_at_modify(prog);
			}
		}
	}
	else if (data[0]=='/'){ // New regex
		auto end=std::find(std::begin(data)+1, std::end(data), '/');
		if (end==std::end(data))
			throw parsing_exception("Invalid regex");
		std::string regex(std::begin(data)+1, end); 
		
		auto prog=std::make_shared<Program>(regex, std::string(end+1, std::end(data)));

		RegexParser::cb_t cb=[prog, regex](Context &ctx){
// 			ctx.output(std::string("Found regex: ")+regex);
			ctx.debug_values();
			prog->run(ctx);
		};
		regex_parser.addRegex( regex, cb);
	}
	else{ // Data
		if (! regex_parser.parse(data, *this) ){
			auto spacepos=data.find_first_of(' ');
			auto key=data.substr(0, spacepos);
			auto value=data.substr(spacepos+1);
			get_value(key).set(to_any(to_number(value)), *this);
		}
	}
}


void Context::set_output(std::function<void (const std::string &data)> &&output)
{
	_output=output;
}


void Context::debug_values()
{
	for (auto &pair: symboltable){
		if (pair.second.get())
			std::cerr<<pair.first<<" = "<<std::to_string(pair.second.get())<<std::endl;
		else
			std::cerr<<pair.first<<std::endl;
	}
}

void Context::output(const std::string& str, const std::string& str2)
{
	output(str+" "+str2);
}

Symbol& Context::get_value(const std::string& key)
{
	auto I=symboltable.find(key);
	if (I!=std::end(symboltable))
		return I->second;
	auto J=symboltable.insert(std::make_pair(key,Symbol(key)));
	
	// Add new dependenies, if matches any old dependency.
	for(auto &kv: glob_dependencies_programs){
		if (glob_match(key, kv.first)){
			J.first->second.run_at_modify(kv.second);
		}
	}
	
	return J.first->second;
}

any Context::fn(const std::string& fname, const std::vector<any> &vars){
	auto F=functions.find(fname);
	if (F==std::end(functions))
		throw std::runtime_error("Unknown function <"+fname+"> called.");
	
	return F->second(*this, vars);
}

any Context::get_glob_values(const std::string& glob){
	std::vector<any> ret;
	// Add new dependenies, if matches any old dependency.
	for(auto &kv: symboltable){
		if (glob_match(kv.first, glob)){
			auto &val=kv.second.get();
			if (val)
				ret.push_back(val->clone());
		}
	}
	return to_any(std::move(ret));
}

std::vector<Symbol*> Context::symboltable_filter(const std::string &glob){
	std::vector<Symbol*> ret;
	for(auto &kv: symboltable){
		if (glob_match(kv.first, glob)){
			Symbol *sym=&kv.second;
			auto &val=sym->get();
			if (val)
				ret.push_back(sym);
		}
	}
	return ret;
}

void Context::register_function(std::string fnname, std::function<any (Context &, const std::vector<any> &)> f)
{
	functions[std::move(fnname)]=f;
}
