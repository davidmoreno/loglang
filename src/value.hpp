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
	using any = std::unique_ptr<value_base>;
}

namespace std{
	std::string to_string(const loglang::any &any);
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
		const std::string type_name;
		
		value_base(std::string _type_name) : type_name(std::move(_type_name)) {} 
		virtual ~value_base(){}
		
		virtual any clone() const = 0;
		virtual int cmp(const any &) const = 0;
		
		virtual int64_t to_int() const{
			throw invalid_conversion("int", this);
		}
		virtual double to_double() const{
			throw invalid_conversion("double", this);
		}
		virtual const std::string &to_string() const{
			throw invalid_conversion("string", this);
		}
		virtual bool to_bool() const{
			throw invalid_conversion("bool", this);
		}
		virtual const std::vector<any> &to_list() const{
			throw invalid_conversion("list", this);
		}
	};
	class string;
	class _int;
	class _double;
	class _bool;
	class _list;
	
	
	any to_any(std::string str);
	any to_any(double val);
	any to_any(int64_t val);
	any to_any(bool val);
	any to_any(std::vector<any> vec);
	
	bool operator==(const any &, const any &);
};
