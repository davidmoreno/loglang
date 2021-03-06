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

#include "program.hpp"
#include "parser.hpp"

namespace loglang{
	extern bool debug;
}

using namespace loglang;

Program::Program(std::string _name, std::string _sourcecode) : name(std::move(_name)), sourcecode(std::move(_sourcecode))
{
	ast=parse_program(sourcecode);
	
	_dependencies=ast->dependencies();
	
	if (debug){
		std::cerr<<name;
// 		std::cerr<<" deps "<<std::to_string(_dependencies);
		std::cerr<<" compiled "<<_sourcecode<<" ast "<<ast->to_string()<<std::endl;
	}
}

void Program::run(Context& context)
{
// 	std::cerr<<"Run "<<name<<std::endl;
	if (ast){
		try{
			auto output=ast->eval(context);
		}
		catch(const std::exception &e){
			std::cerr<<"ERROR running "<< name <<": "<<e.what()<<std::endl;
		}
// 		context.output(output);
	}
}
