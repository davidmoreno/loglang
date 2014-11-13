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

#include "dataitem.hpp"
#include "program.hpp"

using namespace loglang;

void DataItem::run_at_modify(std::shared_ptr< Program > _at_modify)
{
	at_modify.push_back(_at_modify);
}

void DataItem::remove_program(std::shared_ptr< Program > _at_modify)
{
	at_modify.erase( std::remove(std::begin(at_modify), std::end(at_modify), _at_modify), std::end(at_modify));
}

std::string DataItem::get()
{
	return value;
}

void DataItem::set(const std::string& str, LogParser &context)
{
	value=str;
	for(auto program: at_modify)
		program->run(context);
}

