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

#pragma once

#include "../types.hpp"

namespace loglang{
	class list_t : public loglang::type_base{
	public:
		using unboxed=std::vector<value>;
		
		list_t();
		AST codegen_f(type_f fid, std::vector< AST > args);
		value create(unboxed v) const;
		value create() const;
		unboxed unbox(const value &v) const;
	};

	extern list_t list_type;
}
