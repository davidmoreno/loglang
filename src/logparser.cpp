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
#include <boost/algorithm/string.hpp>

#include "logparser.hpp"
#include "program.hpp"
#include "glob.hpp"
#include "utils.hpp"

using namespace loglang;

LogParser::LogParser()
{
	_output=[](const std::string &str){
		std::cout<<str<<std::endl;
	};
}

void LogParser::feed(const std::string& _data)
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
		boost::algorithm::trim(data);
	if (data.length()==0)
		return;
	
	if (data.length()>0 && data[0]==':'){ // New program
		auto colonpos=data.find_first_of(' ');
		auto key=data.substr(0, colonpos);
		if (colonpos>=data.length()){ // Remove, no program
			auto prog=programs[key];
			for (auto &dep: prog->dependencies()){
				get_value(dep).remove_program(prog);
				
				auto I=std::begin(datastore_glob), endI=std::end(datastore_glob);
				for(; I != endI; ) {
					if (I->second==prog) {
							datastore_glob.erase(I++);
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
					for (auto &key_value: datastore){
						if (glob_match(key_value.first, dep)){
// 							std::cerr<<"Match!"<<std::endl;
							get_value(key_value.first).run_at_modify(prog);
						}
					datastore_glob[dep]=prog;
					}
				}
				else // No glob
					get_value(dep).run_at_modify(prog);
			}
		}
	}
	else{ // Data
		auto spacepos=data.find_first_of(' ');
		auto key=data.substr(0, spacepos);
		auto value=data.substr(spacepos+1);
		get_value(key).set(to_any(to_number(value)), *this);
	}
}


void LogParser::set_output(std::function<void (const std::string &data)> &&output)
{
	_output=output;
}


void LogParser::debug_values()
{
	for (auto &pair: datastore){
		std::cerr<<pair.first<<" = "<<std::to_string(pair.second.get())<<std::endl;
	}
}

void LogParser::output(const std::string& str, const std::string& str2)
{
	output(str+" "+str2);
}

DataItem& LogParser::get_value(const std::string& key)
{
	auto I=datastore.find(key);
	if (I!=std::end(datastore))
		return I->second;
	auto J=datastore.insert(std::make_pair(key,DataItem(key)));
	
	// Add new dependenies, if matches any old dependency.
	for(auto &kv: datastore_glob){
		if (glob_match(key, kv.first)){
			J.first->second.run_at_modify(kv.second);
		}
	}
	
	return J.first->second;
}

any LogParser::fn(const std::string& fname, const std::vector<any> &vars){
	if (fname=="sum"){
		double n=0.0;
		for(auto &v: vars){
			try{
				auto &operands=v->to_list();
				for (auto &op: operands)
					n+=op->to_double();
			}
			catch(const value_base::invalid_conversion &e){
				n+=v->to_double();
			}
		}
		return to_any( n );
	}
	else if (fname=="print"){
		auto &dataitem=get_value(vars[0]->to_string());
		output(vars[0]->to_string(), std::to_string( dataitem.get() ));
		return dataitem.get()->clone();
	}
	else if (fname=="round"){
// 		std::cerr<<"round"<<std::endl;
		auto dataitem=vars[0]->to_double();
		auto ndig=vars[1]->to_double();
		double mult=pow(10, ndig);
// 		std::cerr<<dataitem<<" "<<mult<<std::endl;
		return to_any( int( dataitem * mult ) / mult );
	}
	else if (fname=="debug"){
		std::cerr<<"DEBUG: ";
		for(auto &v: vars)
			std::cerr<<std::to_string(v)<<" ";
		std::cerr<<std::endl;
		return to_any( true );
	}
	throw std::runtime_error("Unknown function <"+fname+"> called.");
}

any LogParser::get_glob_values(const std::string& glob){
	std::vector<any> ret;
	// Add new dependenies, if matches any old dependency.
	for(auto &kv: datastore){
		if (glob_match(kv.first, glob)){
			auto &val=kv.second.get();
			if (val)
				ret.push_back(val->clone());
		}
	}
	return to_any(std::move(ret));
}

