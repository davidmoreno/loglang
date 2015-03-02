/*
 * Copyright 2014-2015 David Moreno
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

#include "types.hpp"


using namespace loglang;

std::unordered_map<type_base::type_id, type,  std::hash<int>> type_base::types;


type_base::type_base(type_base::type_id t){
	type_base::types[t]=this;
}


type type_base::get(type_base::type_id tid)
{
	auto r=types.find(tid);
	if (r==std::end(types))
		throw std::runtime_error("Unknown type");
	return r->second;
}

unknown_function::unknown_function(type t, type_f f)
{
	switch(f){
		case type_f::ADD:
			fname="+";
			break;
		case type_f::SUB:
			fname="-";
			break;
		case type_f::NONE:
			fname="<none>";
			break;
		case type_f::LT:
			fname="<";
			break;
		case type_f::GT:
			fname=">";
			break;
		case type_f::LTE:
			fname="<=";
			break;
		case type_f::GTE:
			fname=">=";
			break;
		case type_f::AND:
			fname="and";
			break;
		case type_f::OR:
			fname="or";
			break;
		case type_f::DIV:
			fname="div";
			break;
		case type_f::MUL:
			fname="mul";
			break;
		case type_f::MOD:
			fname="mod";
			break;
		case type_f::IN:
			fname="in";
			break;
		case type_f::TO_STRING:
			fname="to_string ";
			break;
		default:
			fname="??";
			break;
	}
	fname=std::string("Unknown function: ")+t->name+":"+fname;
}
