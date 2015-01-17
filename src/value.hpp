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
	class value_base;
	class type_base;
	using value = std::unique_ptr<value_base>;
}

namespace std{
	std::string to_string(const loglang::value &any);
	std::string to_string(const loglang::value_base *any);
}

namespace loglang{
	class value_base{
	public:
		class invalid_conversion : public std::exception {
			std::string str;
		public:
			invalid_conversion(std::string _str, const value_base *v){
				str="Invalid conversion to ";
				str+=_str;
				str+=" from ";
				str+=std::to_string(v);
			}
			const char* what() const throw() override{
				return str.c_str();
			}
		};
	public:
		type_base *type;
		
		value_base(type_base *t) : type(t) {} 
		virtual ~value_base(){}
		
		virtual value clone() const = 0;
	};
};
