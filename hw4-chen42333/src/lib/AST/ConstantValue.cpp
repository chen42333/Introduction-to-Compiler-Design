#include "AST/ConstantValue.hpp"
#include <string.h>

ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, int value, Type t)
    : ExpressionNode{line, col, t}, integer(value){
        sprintf(type, "integer");
    }
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, double value, Type t)
    : ExpressionNode{line, col, t}, floating(value){
        sprintf(type, "real");
    }
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, const char* value, Type t)
    : ExpressionNode{line, col, t}{
        //constType = String;
        sprintf(type, "string");
        str = value;
    }
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, bool value, Type t)
    : ExpressionNode{line, col, t}, boolean(value){
            sprintf(type, "boolean");
    }

char* ConstantValueNode::getValue(){
    if(strcmp(type, "integer") == 0) sprintf(type_str, "%d", integer);
    else if(strcmp(type, "real") == 0) sprintf(type_str, "%lf", floating);
    else if(strcmp(type, "string") == 0) sprintf(type_str, "%s", str);
    else if(strcmp(type, "boolean") == 0){
        if(boolean) sprintf(type_str, "true");
        else sprintf(type_str, "false");
    }
    return type_str;
}
