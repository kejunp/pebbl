/**
 * @file main.cpp
 * @brief Main entry point for the PEBBL language interpreter
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "lexer.hpp"
#include "ast_generator.hpp"
#include "interpreter.hpp"
#include "gc.hpp"

void run_code(const std::string& source) {
    try {
        // Create GC heap
        GCHeap heap;
        
        // Parse the source code
        Lexer lexer{std::string(source)};
        ASTGenerator generator(lexer);
        auto program = generator.parse_program();
        
        if (!program) {
            std::cerr << "Failed to parse program" << std::endl;
            return;
        }
        
        // Execute the program
        Interpreter interpreter(heap);
        auto result = interpreter.execute(*program);
        
        // Print result if it's not null
        if (!result.is_null()) {
            std::cout << interpreter.stringify(result) << std::endl;
        }
        
    } catch (const RuntimeError& e) {
        // Runtime errors are already printed by the interpreter
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void run_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    run_code(source);
}

void run_repl() {
    std::cout << "PEBBL Interactive Interpreter" << std::endl;
    std::cout << "Type 'exit' to quit" << std::endl;
    std::cout << std::endl;
    
    GCHeap heap;
    Interpreter interpreter(heap);
    
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break; // EOF
        }
        
        if (line == "exit" || line == "quit") {
            break;
        }
        
        if (line.empty()) {
            continue;
        }
        
        try {
            Lexer lexer{std::string(line)};
            ASTGenerator generator(lexer);
            auto program = generator.parse_program();
            
            if (program) {
                auto result = interpreter.execute(*program);
                if (!result.is_null()) {
                    std::cout << interpreter.stringify(result) << std::endl;
                }
            }
        } catch (const RuntimeError& e) {
            // Runtime errors are already printed by the interpreter
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

void test_interpreter() {
    std::cout << "Testing PEBBL Interpreter" << std::endl;
    std::cout << "=========================" << std::endl;
    
    std::vector<std::string> test_cases = {
        "42;",
        "3.14;", 
        "\"hello world\";",
        "true;",
        "1 + 2;",
        "5 * 3 - 2;",
        "let x = 10; x;",
        "let y = 5; y * 2;",
        "[1, 2, 3];",
        "{\"name\": \"Alice\", \"age\": 25};",
        "if true { 42 } else { 0 };",
        "let a = 5; let b = 10; a + b;"
    };
    
    for (const auto& test : test_cases) {
        std::cout << ">>> " << test << std::endl;
        std::string test_copy = test;  // Make a copy to avoid const issues
        run_code(test_copy);
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // No arguments - run REPL
        run_repl();
    } else if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "--test") {
            test_interpreter();
        } else if (arg == "--repl") {
            run_repl();
        } else {
            // Assume it's a filename
            run_file(arg);
        }
    } else {
        std::cout << "Usage: " << argv[0] << " [--test|--repl|filename]" << std::endl;
        std::cout << "  --test    : Run interpreter tests" << std::endl;
        std::cout << "  --repl    : Run interactive REPL" << std::endl;
        std::cout << "  filename  : Execute a PEBBL source file" << std::endl;
        std::cout << "  (no args) : Start interactive REPL" << std::endl;
        return 1;
    }
    
    return 0;
}