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

#include <sstream>
#include <iostream>

#include "value.hpp"
#include "utils.hpp"
#include "cxxabi.h"



/// Implementation of types.

namespace loglang{
	class string : public value_base{
		std::string str;
	public:
		string(std::string str) : str(std::move(str)) {}
		const std::string &to_string() const override{
			return str;
		}
		std::unique_ptr< value_base > clone() const override{
			return to_any(str);
		}
		int cmp(const any &o) const{
			return str.compare( o->to_string() );
		}
	};
	class _double : public value_base{
		double val;
	public:
		_double(double d) : val(d) {}
		virtual double to_double() const override{
			return val;
		}
		std::unique_ptr< value_base > clone() const override{
			return to_any(val);
		}
		int cmp(const any &o) const{
			auto r=val - o->to_double();
// 			std::cerr<<"cmp d"<<val<<" >< "<<o->to_double()<<" = "<<r<<std::endl;
			return (r<0) ? -1 : (r>0) ? 1 : 0;
		}
	};
	class _int : public value_base{
		int64_t val;
	public:
		_int(int64_t d) : val(d) {}
		virtual int64_t to_int() const override{
			return val;
		}
		virtual double to_double() const override{
			return val;
		}
		std::unique_ptr< value_base > clone() const override{
			return to_any(val);
		}
		
		int cmp(const any &o) const{
			return val - o->to_int();
		}
	};
	class _bool : public value_base{
		bool val;
	public:
		_bool(bool d) : val(d) {}
		std::unique_ptr< value_base > clone() const override{
			return to_any(val);
		}
		virtual bool to_bool() const override{
			return val;
		}
		int cmp(const any &o) const{
			return val!=o->to_bool();
		}
	};
	class _list : public value_base{
		std::vector<any> val;
	public:
		_list(std::vector<any> d) : val(std::move(d)) {}
		std::unique_ptr< value_base > clone() const override{
			std::vector<any> copy;
			for(auto &a: val)
				copy.push_back(a->clone());
			return to_any(std::move(copy));
		}
		virtual const std::vector<any> &to_list() const override{
			return val;
		}
		int cmp(const any &o) const{
			const _list *po=dynamic_cast<const _list *>(o.get());
			auto I=std::begin(val), endI=std::end(val);
			auto J=std::begin(po->val), endJ=std::end(po->val);
			while (I!=endI && J!=endJ){
				auto res=(*I)->cmp(*J);
				if (res!=0)
					return res;
				++I; ++J;
			}
			if (I==endI && J==endJ)
				return 0;
			if (I!=endI)
				return -1;
			if (J!=endJ)
				return 1;
			return -100; // Should never get here.
		}
	};
}

// Other functions.

std::string std::to_string(const loglang::value_base *any);

std::string std::to_string(const loglang::any &any)
{
	return std::to_string(static_cast<const loglang::value_base *>(any.get()));
}


std::string std::to_string(const loglang::value_base *any){
	{
		auto val=dynamic_cast<const loglang::string*>(any);
		if (val)
			return std::string("\"")+val->to_string()+std::string("\"");
	}
	{
		auto val=dynamic_cast<const loglang::_int*>(any);
		if (val)
			return std::to_string(val->to_int());
	}
	{
		auto val=dynamic_cast<const loglang::_double*>(any);
		if (val)
			return std::to_string(val->to_double());
	}
	{
		auto val=dynamic_cast<const loglang::_bool*>(any);
		if (val)
			return val->to_bool() ? "true" : "false";
	}
	{
		auto val=dynamic_cast<const loglang::_list*>(any);
		if (val){
			std::stringstream ret;
			ret<<"[";
			bool first=true;
			for(auto &v: val->to_list()){
				if (!first){
					ret<<", ";
				}
				else
					first=false;
				ret<<std::to_string(v);
			}
			ret<<"]";
			return ret.str();
		}
	}
	
	std::string tpename=typeid(*any).name();
	int st;
	char *demangled=abi::__cxa_demangle(tpename.c_str(), 0, 0, &st);
	if (demangled){
		tpename=demangled;
		free(demangled);
	}

	return std::string("[[")+tpename+"]]";
}

using namespace loglang;

any loglang::to_any(std::string str){
	return std::make_unique<string>(std::move(str));
}

any loglang::to_any(double val){
	return std::make_unique<_double>(val);
}

any loglang::to_any(int64_t val){
	return std::make_unique<_int>(val);
}

any loglang::to_any(bool val){
	return std::make_unique<_bool>(val);
}
any loglang::to_any(std::vector<any> val){
	return std::make_unique<_list>(std::move(val));
}


namespace loglang{
	bool operator==(const any &a, const any &b){
		if (!a || !b) // null elements always false.
			return false;
		
		if (typeid(*a)!=typeid(*b))
			return false;
// 		std::cerr<<"Cmp"<<std::endl;
		return a->cmp(b) == 0;
	}
};
