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

#include <set>
#include <string>
#include <sstream>
#include <memory>

namespace std{
	template<typename T=std::string>
	std::string to_string(std::string s){
		return s;
	}
	
	template<typename T>
	std::string to_string(const T &set){
		std::stringstream ss;
		bool first=true;
		ss<<"{";
		for(auto &s: set){
			if (!first)
				ss<<", ";
			else
				first=false;
			ss<<std::to_string(s);
		}
		ss<<"}";
		return ss.str();
	}

	template<typename _Tp>
	struct _MakeUniq
	{ typedef unique_ptr<_Tp> __single_object; };


	template<typename _Tp, typename... _Args>
	inline typename _MakeUniq<_Tp>::__single_object
	make_unique(_Args&&... __args)
	{ return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...)); }


	class runtime_error : public std::exception{
		std::string str;
	public:
		runtime_error(std::string _str) : str(std::move(str)){}
		const char *what() const throw() override{ return str.c_str(); }
	};
};


namespace loglang{
	void print_backtrace();
	double to_number(const std::string &str);
	void trim(std::string &);
};

