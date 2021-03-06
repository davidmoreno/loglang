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

namespace loglang{
	extern bool debug;
}

using namespace loglang;

Context::Context()
{
	_output=[](const std::string &str){
		std::cout<<str<<std::endl;
	};
	
	register_builtins(*this);
}

void Context::feed_secure(std::string data)
{
	if (data.length()>0 && data[0]==':'){ // New program
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
	else{ // Data
		feed(data);
	}
}

void Context::feed(std::string data){
	::loglang::clean(data);
	if (data.length()==0)
		return;
	auto spacepos=data.find_first_of(' ');
	auto key=data.substr(0, spacepos);
	auto value=data.substr(spacepos+1);
	if (debug){
		std::cerr<<"Set <"<<key<<"> = <"<<value<<">"<<std::endl;
	}
	get_value(key).set(to_any(int64_t(to_number(value))), *this);
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
