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
#include <set>
#include <memory>

namespace loglang{
	class Context;
	class ASTBase;
	class JITProgram;
	
	class Program{
		std::string name;
		std::string sourcecode;
		std::set<std::string> _dependencies;
		JITProgram *jit;
		
	public:
		Program(std::string name, std::string sourcecode, Context &context);
		~Program();
		const std::set<std::string> &dependencies() const { return _dependencies; }
		
		void run(Context &context);
	};
}
