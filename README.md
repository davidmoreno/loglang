# Log Lang

Its a rule based programming language. If the dependencies change, then the rules are executed, which in turn can execute another rule.

Data and code goes thought the same input, which may be a security problem.

# Pseudo BNF

	Program -> []
	[OP] -> + - / * < > <= >= == in and or
	[] -> var = []
	[] -> [] [OP] []
	[] -> if [] then [] else []
	[] -> edge_if [] then [] else []
	[] -> at [] do []
	[] -> [const]
	[] -> [var]
	[] -> '[' (([],)* []|) ']' # list
	[] -> { (([];)* []|) } # block, returns last value
	[] -> [func]( (([],)* []|) ) 
	[func] -> sum | print | ...


Some remarks:

* edge_if only executes when The condition changes the value.
* at fires when [] changes. 
* edge_if can be composed as a "at [A] if [A] then ... else ..."
* var can be anything that starts with a letter, and letter, digit or %.- follows. If it does not exist, its created with "" value.
* var can contain a glob, which makes it apply inmediatly on current symbols, and dynamically on new added symbols. It expands as a list of vars.
* func is a builtin func
* When running a rule, there is a reference to current changed value that triggered the rule as %.

# Builtins

* sum( list )  -- Sum of the given list values
* print( var ) -- Prints the name and value of the given 

# Programs vs data

Programs start with :id, and then the code. Data is the data id, and the value. Comments are lines starting with #, and ignored to the end.

When the value changes all depending codes are executed; they can be implicit as in plain asignments or explicit as in "at", "if" and "edge_if".


# Example

:mem.free%   mem.free% = ( mem.free + mem.cached ) * 100.0 / mem.total
:cpu.cpu%    cpu.cpu% = 100 - ( ( cpu.cpu.nice + cpu.cpu.idle ) * 100.0 / ( sum cpu.cpu.* ) )
:dsk.read    at timestamp do { disk.read_total_0 = disk.read_total ; disk.read_total = ( sum disk.*.read ) ; disk.read = ( disk.read_total - disk.read_total_0 ) / (timestamp - timestamp_0) ) }
mem.free 10
mem.total 100
