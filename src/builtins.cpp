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

#include "builtins.hpp"
#include "context.hpp"

namespace loglang{
	namespace builtins{
		static any sum(Context&, const std::vector<any> &vars){
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
		static any print(Context &context, const std::vector<any> &vars){
			auto symlist=context.symboltable_filter(vars[0]->to_string());
			for (auto sym: symlist){
				context.output(sym->name(), std::to_string( sym->get() ));
			}
			return to_any( (int64_t)vars.size() );
		}
		static any round(Context &context, const std::vector<any> &vars){
			auto dataitem=vars[0]->to_double();
			auto ndig=vars[1]->to_double();
			double mult=pow(10, ndig);
	// 		std::cerr<<dataitem<<" "<<mult<<std::endl;
			return to_any( int( dataitem * mult ) / mult );
		}

		static any debug(Context &context, const std::vector<any> &vars){
			std::cerr<<"DEBUG: ";
			for(auto &v: vars)
				std::cerr<<std::to_string(v)<<" ";
			std::cerr<<std::endl;
			return to_any( true );
		}
		
		
		static any to_int(Context &context, const std::vector<any> &vars){
			if (vars.size()!=1)
				throw std::runtime_error("Invalid number of arguments for to_int.");
			return to_any(int64_t(atoi(vars[0]->to_string().c_str())));
		}
	}
	
}

void loglang::register_builtins(loglang::Context& context){
	context.register_function("sum", &loglang::builtins::sum);
	context.register_function("print", &loglang::builtins::print);
	context.register_function("round", &loglang::builtins::round);
	context.register_function("debug", &loglang::builtins::debug);
	context.register_function("to_int", &loglang::builtins::to_int);
}
