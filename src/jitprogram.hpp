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

#include <string>
#include "ast.hpp"

namespace llvm{
	class Function;
}

namespace loglang{
	class JITProgram{
		llvm::Function *llvm_function;
	public:
		JITProgram(const std::string &program_name, const AST &root_node);
		~JITProgram();
		
		void run(any &);
	};
};
