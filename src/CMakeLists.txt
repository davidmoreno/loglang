add_executable(loglang 
	types.cpp value.cpp
	main.cpp utils.cpp parser.cpp context.cpp symbol.cpp program.cpp tokenizer.cpp glob.cpp builtins.cpp
	)

install(TARGETS loglang RUNTIME DESTINATION bin)
target_link_libraries(loglang loglang_types)

add_subdirectory(types)