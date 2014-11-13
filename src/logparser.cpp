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

#include "logparser.hpp"
#include "program.hpp"

using namespace loglang;

LogParser::LogParser()
{
	_output=[](const std::string &str){
		std::cout<<">> "<<str<<std::endl;
	};
}

void LogParser::feed(const std::string& data)
{
	if (data.length()>0 && data[0]==':'){ // New program
		auto colonpos=data.find_first_of(' ');
		auto key=data.substr(0, colonpos);
		if (colonpos>=data.length()){ // Remove, no program
			auto prog=programs[key];
			for (auto &dep: prog->dependencies()){
				get_value(dep).remove_program(prog);
			}
			programs.erase(key);
		}
		else{
			auto value=data.substr(colonpos+1);
			auto prog=std::make_shared<Program>(std::move(value));
			
			programs[key]=prog;
			for (auto &dep: prog->dependencies()){
				get_value(dep).run_at_modify(prog);
			}
		}
	}
	else{ // Data
		auto spacepos=data.find_first_of(' ');
		auto key=data.substr(0, spacepos);
		auto value=data.substr(spacepos+1);
		output(key+" "+value);
		get_value(key).set(value, *this);
	}
}


void LogParser::set_output(std::function<void (const std::string &data)> &&output)
{
	_output=_output;
}


void LogParser::debug_values()
{
	for (auto &pair: datastore){
		std::cout<<pair.first<<" = "<<pair.second.get()<<std::endl;
	}
}
