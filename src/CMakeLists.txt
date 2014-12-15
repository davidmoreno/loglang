add_executable(loglang main.cpp utils.cpp parser.cpp context.cpp symbol.cpp 
	program.cpp tokenizer.cpp glob.cpp value.cpp builtins.cpp regexparser.cpp)

target_link_libraries(loglang pcre)

install(TARGETS loglang RUNTIME DESTINATION bin)

