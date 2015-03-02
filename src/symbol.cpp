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

#include <algorithm>

#include "symbol.hpp"
#include "program.hpp"
#include "types.hpp"
#include "context.hpp"
#include "types/string.hpp"

using namespace loglang;

Symbol::Symbol(std::string name) : _name(std::move(name))
{

}


void Symbol::run_at_modify(std::shared_ptr< Program > _at_modify)
{
	at_modify.push_back(_at_modify);
}

void Symbol::remove_program(std::shared_ptr< Program > _at_modify)
{
	at_modify.erase( std::remove(std::begin(at_modify), std::end(at_modify), _at_modify), std::end(at_modify));
}

const loglang::value &Symbol::get() const
{
	return val;
}

void Symbol::set(value new_val, Context &context)
{
	if (val==new_val) // Ignore no changes.
		return; 
	val=std::move(new_val);
// 	context.output(name, value);
	if (_name=="%") // Prevent recursion.
		return;
	context.get_value("%").set(string_type.create(_name), context);
	for(auto program: at_modify)
		program->run(context);
}
