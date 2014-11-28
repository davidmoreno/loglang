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

	class value_base{
	public:
		class invalid_conversion : public std::exception {};
	public:
		virtual ~value_base(){}
		
		virtual any clone() const = 0;
		virtual int cmp(const any &) const = 0;
		
		virtual int64_t to_int() const{
			throw invalid_conversion();
		}
		virtual double to_double() const{
			throw invalid_conversion();
		}
		virtual const std::string &to_string() const{
			throw invalid_conversion();
		}
		virtual bool to_bool() const{
			throw invalid_conversion();
		}
		virtual const std::vector<any> &to_list() const{
			throw invalid_conversion();
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

namespace std{
	std::string to_string(const ::loglang::any &any);
}
