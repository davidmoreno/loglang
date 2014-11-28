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

#include "value.hpp"

namespace loglang{
	class Context;
	class ASTBase{
	public:
		virtual any eval(Context &context) = 0;
		virtual std::string to_string() = 0;
		virtual std::set<std::string> dependencies() = 0;
	};
	
	using AST=std::unique_ptr<ASTBase>;
}
