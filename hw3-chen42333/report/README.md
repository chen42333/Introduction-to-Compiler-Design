# hw3 report

|||
|-:|:-|
|Name|陳芷萱|
|ID|110550029|

## How much time did you spend on this project

About 24 hours.

## Project overview

- **Scanner**: Let all tokens return the corresponding terminals defined in the parser, and let identifiers, integers, reals, scientific notations, strings, operators and data types return their values. The octal integers will be convert to decimal intergers before returning, and the boolean constants don't need to return their value since the parser will directly give the corresponding value once it sees the terminal ```TRUE``` or ```FALSE```.
- **Parser**: 
    - Check the types that each symbol will return, and then define them with ```%type```, and add the necessary types into ```%union``` section. 
    - If the type is vector or class, store its pointer rather than directly store its value. 
    - If some symbol have to return more than one type, create a struct that contains all needed types for that symbol. Ex: 
        - ```Type```, ```ArrType```: a string and a list of numbers
        - ```IdList```: a list of strings and two lists of numbers (to store the locations of the identifiers)
        - ```Simple```: a ```AssignmentNode``` pointer, a ```PrintNode``` pointer and a ```ReadNode``` pointer
    - An action of a rule will create a new node with specified information if needed.
- **Nodes**: 
    - Visitor pattern is used to dump the AST in this project. There is a dumper visits all nodes from the root rather than all the nodes directry print their information and then call their children to print. Everytime a node will "accept" the dumper, and the dumper visits this node afterwards. The procedure of visiting is to print the information of the node first, and then call the visit child function of that node unless it's a constant value node, which must be a leaf node. And since visitor pattern is used, the nodes don't have to print their information by themselves, so the ```print``` function was removed from all the node classes, including the base class (```AstNode```).
    - There are some nodes having a list of experssion nodes or a list of statement nodes; however, expression nodes and statement nodes are actually classes of node types rather than classes of nodes, and a vector or any container in C/C++ can only contain data with the same type. So the nodes will be upcasted to an ```ExpressionNode``` for an expression nodes list and upcasted to an ```AstNode``` for a statement nodes list. To specify their original types later, every node has a enum type member variable ```type``` (defined in their base class ```AstNode``` with ```getType()```), which will be defined when the node is created. Before the node being visited, its parent will downcast the node according to its ```type```, and then pass the dumper to the node.
    - All node classes were implemented with the following procedure: 
        - Include the header of visitor and all the node types it will reference.
        - Check the information it needs and add it to the member variables, design the constructor (may be more than one).
        - Implement virtual function ```accept()``` and ```visitChildNodes()```.
        - Check ```AstDumper.cpp``` to see whether the dumper needs some its getters to print out its information, and implement the needed getters.
        - Write actions in the parser to create the nodes.
    - The information needed to create a new node of the head symbol are directly provided by the symbols in the body for the most part. However, some information needed by a ```ForNode``` are not provided by the symbols in the body, so it first create the needed nodes and data from the information provided by the symbols in the body, and then use them to create a ```ForNode```.

## What is the hardest you think in this project
I think the hardest part is to pass the second testcase. The first testcase can be done by following the hint, but it has to complete almost one third to a half of this project to pass the second testcase, and it also made me take a large amount of time to debug. But the later testcases are not difficult if passed the second testcase.

## Feedback to T.A.s
I think the spec and other documents are too long. I took a lot of time to read them but still cannot completely realilze what I have to do until I tried the demonstrates in hint.md by myself. But I think the hints (in the documents or the comments in the code) are clear and helpful after I starting coding.
