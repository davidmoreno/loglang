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
#include <math.h>

#include "types.hpp"
#include "builtins.hpp"
#include "context.hpp"
#include "types/int.hpp"
#include "types/string.hpp"
#include "types/double.hpp"
#include "types/list.hpp"
#include "types/bool.hpp"

namespace loglang{
	namespace builtins{
		static value sum(Context&, const std::vector<value> &vars){
			double n=0.0;
			for(auto &v: vars){
				auto operands=list_type.unbox(v);
				for (auto &op: operands)
					n+= double_type.to_double(op);
			}
			return double_type.create( n );
		}
		static value print(Context &context, const std::vector<value> &vars){
			auto symlist=context.symboltable_filter( string_type.to_string( vars[0] ));
			for (auto sym: symlist){
				context.output(sym->name(), std::to_string( sym->get() ));
			}
			return int_type.create( (int64_t)vars.size() );
		}
		static value round(Context &context, const std::vector<value> &vars){
			auto dataitem=double_type.to_double( vars[0] );
			auto ndig=double_type.to_double( vars[1] );
			double mult=pow(10, ndig);
	// 		std::cerr<<dataitem<<" "<<mult<<std::endl;
			return double_type.create( int( dataitem * mult ) / mult );
		}

		static value debug(Context &context, const std::vector<value> &vars){
			std::cerr<<"DEBUG: ";
			for(auto &v: vars)
				std::cerr<<string_type.to_string( v )<<" ";
			std::cerr<<std::endl;
			return bool_type.create( true );
		}
		
	}
	
}

void loglang::register_builtins(loglang::Context& context){
	context.register_function("sum", &loglang::builtins::sum);
	context.register_function("print", &loglang::builtins::print);
	context.register_function("round", &loglang::builtins::round);
	context.register_function("debug", &loglang::builtins::debug);
}
