#include "AST/variable.hpp"

VariableNode::VariableNode(const uint32_t line, const uint32_t col, char* name, char* type, ConstantValueNode* const_val,  std::vector<int> *array_size_list, Type t)
    : AstNode{line, col, t}, name(name), type(type), const_val(const_val) {
        if(array_size_list != NULL) this->array_size_list = *array_size_list;
    }

void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if(const_val != NULL) const_val->accept(p_visitor);
}

char* VariableNode::getType(){
    sprintf(type_str, "%s", type);
    if(array_size_list.size() > 0) sprintf(type_str, "%s ", type_str);
    for(int i: array_size_list) sprintf(type_str, "%s[%d]", type_str, i);
    return type_str;
}
char* VariableNode::getName(){
    return name;
}

char* VariableNode::getValue(){
    if (const_val == NULL) return NULL;
    return const_val->getValue();
}