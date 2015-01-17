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

#include <string>
#include <algorithm>

#include <execinfo.h>
#include <iostream>

#include "utils.hpp"
#include "cxxabi.h"

namespace loglang {
	/// Inspiration from http://www.gnu.org/software/libc/manual/html_node/Backtraces.html
	void print_backtrace()
	{
		void *array[10];
		size_t size;
		char **strings;
		size_t i;

		size = backtrace (array, 10);
		strings = backtrace_symbols (array, size);

		std::cerr<<"Obtained "<<size<<" stack frames."<<std::endl;

		int st;
		for (i = 0; i < size; i++){
			std::string fname=strings[i];
			auto s=std::find(std::begin(fname), std::end(fname), '(')+1;
			auto e=std::find(std::begin(fname), std::end(fname), '+');
			fname=std::string(s, e);
			
			char *demangled=abi::__cxa_demangle(fname.c_str(), 0, 0, &st);
			if (demangled){
				std::cerr<<demangled<<std::endl; //std::string(std::begin(fname),s)<<demangled<<std::string(s, std::end(fname)-1)<<std::endl;
				free(demangled);
			}
			else{
				std::cerr<<strings[i]<<std::endl;
			}
		}

		free (strings);
	}

	double to_number(const std::string &str){
		return std::atof(str.c_str());
	}

	void trim(std::string &str){
		if (str.length()==0)
			return;
		auto begin=std::find_if(std::begin(str), std::end(str), [](char c){ return !isspace(c); });
		auto end=std::find_if(str.rbegin(), str.rend(), [](char c){ return !isspace(c); });
		str=std::string( begin, end.base() );
	}
	
	/// Cleans a line of comments and empt leading and ending spaces.
	void clean(std::string &data){
		auto comment=std::find(std::begin(data), std::end(data), '#');
		if (comment!=std::end(data)){ // Remove comments
			data.erase(comment, std::end(data));
		}
		if (std::isspace(data[0]))
			trim(data);
	}
	
	/// Adds two sets. First as lref to use thrown aways.
	std::set< std::string > operator+(std::set< std::string > s1, const std::set< std::string >& s2)
	{
		for(auto &s: s2)
			s1.insert(s);
		return  s1;
	}
}

