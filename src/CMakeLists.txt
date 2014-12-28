add_executable(loglang 
	main.cpp utils.cpp parser.cpp context.cpp symbol.cpp program.cpp jitprogram.cpp 
	tokenizer.cpp glob.cpp value.cpp builtins.cpp)

install(TARGETS loglang RUNTIME DESTINATION bin)
target_link_libraries(loglang -L/usr/lib64/llvm -lLLVM-3.5)

