#include <iostream>
#include <string>
#include "lexer.hpp"
#include "ast_generator.hpp"

void test_array_parsing() {
    std::cout << "Testing Array Parsing:" << std::endl;
    
    try {
        // Test simple array in expression statement
        Lexer lexer("[1, 2, 3];");
        ASTGenerator generator(lexer);
        auto result = generator.parse_program();
        
        if (result && !result->statements.empty()) {
            auto stmt = result->statements[0].get();
            if (stmt->type() == ASTType::EXPRESSION_STATEMENT) {
                auto expr_stmt = static_cast<ExpressionStatementNode*>(stmt);
                if (expr_stmt->expression->type() == ASTType::ARRAY_LITERAL) {
                    auto array = static_cast<ArrayLiteralNode*>(expr_stmt->expression.get());
                    std::cout << "  Array with " << array->elements.size() << " elements: SUCCESS" << std::endl;
                } else {
                    std::cout << "  Array parsing: FAILED (wrong expression type)" << std::endl;
                }
            } else {
                std::cout << "  Array parsing: FAILED (wrong statement type)" << std::endl;
            }
        } else {
            std::cout << "  Array parsing: FAILED (no statements)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "  Array parsing: FAILED (exception: " << e.what() << ")" << std::endl;
    }
}

void test_dict_parsing() {
    std::cout << "Testing Dictionary Parsing:" << std::endl;
    
    try {
        // Test simple dict in expression statement
        Lexer lexer("{\"name\": \"John\", \"age\": 30};");
        ASTGenerator generator(lexer);
        auto result = generator.parse_program();
        
        if (result && !result->statements.empty()) {
            auto stmt = result->statements[0].get();
            if (stmt->type() == ASTType::EXPRESSION_STATEMENT) {
                auto expr_stmt = static_cast<ExpressionStatementNode*>(stmt);
                if (expr_stmt->expression->type() == ASTType::DICT_LITERAL) {
                    auto dict = static_cast<DictLiteralNode*>(expr_stmt->expression.get());
                    std::cout << "  Dict with " << dict->entries.size() << " entries: SUCCESS" << std::endl;
                } else {
                    std::cout << "  Dict parsing: FAILED (wrong expression type)" << std::endl;
                }
            } else {
                std::cout << "  Dict parsing: FAILED (wrong statement type)" << std::endl;
            }
        } else {
            std::cout << "  Dict parsing: FAILED (no statements)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "  Dict parsing: FAILED (exception: " << e.what() << ")" << std::endl;
    }
}

int main() {
  std::cout << "Testing Array and Dictionary Parsing" << std::endl;
  std::cout << "====================================" << std::endl;
  
  test_array_parsing();
  test_dict_parsing();
  
  return 0;
}