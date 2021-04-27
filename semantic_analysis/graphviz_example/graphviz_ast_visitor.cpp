//
// Created by Xandru Mifsud on 29/03/2021.
//

#include "graphviz_ast_visitor.h"

graphviz_ast_visitor::graphviz_ast_visitor(){
    outfile.open("/Users/xandrumifsud/Documents/Dev/CPS2000/CPS2000/ast.dot");
    outfile << "digraph ast {" << std::endl;
}

void graphviz_ast_visitor::visit(astTYPE *node){}
void graphviz_ast_visitor::visit(astLITERAL *node){}
void graphviz_ast_visitor::visit(astIDENTIFIER *node){}

void graphviz_ast_visitor::visit(astMULTOP *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->operand1->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->operand2->getLabel() << "\"" << std::endl;

    node->operand1->accept(this);
    node->operand2->accept(this);
}

void graphviz_ast_visitor::visit(astADDOP *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->operand1->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->operand2->getLabel() << "\"" << std::endl;

    node->operand1->accept(this);
    node->operand2->accept(this);
}

void graphviz_ast_visitor::visit(astRELOP *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->operand1->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->operand2->getLabel() << "\"" << std::endl;

    node->operand1->accept(this);
    node->operand2->accept(this);
}

void graphviz_ast_visitor::visit(astAPARAMS *node){
    for(auto &c : *node->children){
        outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << c->getLabel() << "\"" << std::endl;
    }

    for(auto &c : *node->children){
        c->accept(this);
    }
}

void graphviz_ast_visitor::visit(astFUNC_CALL *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    if(node->aparams != nullptr){ outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->aparams->getLabel() << "\"" << std::endl;}

    node->identifier->accept(this);
    if(node->aparams != nullptr){ node->aparams->accept(this);}
}

void graphviz_ast_visitor::visit(astSUBEXPR *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->subexpr->getLabel() << "\"" << std::endl;
    node->subexpr->accept(this);
}

void graphviz_ast_visitor::visit(astUNARY *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->operand->getLabel() << "\"" << std::endl;
    node->operand->accept(this);
}

void graphviz_ast_visitor::visit(astASSIGNMENT *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    
    node->identifier->accept(this);
    node->expression->accept(this);
}

void graphviz_ast_visitor::visit(astVAR_DECL *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->type->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    
    node->identifier->accept(this);
    node->type->accept(this);
    node->expression->accept(this);
}

void graphviz_ast_visitor::visit(astPRINT *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    node->expression->accept(this);
}

void graphviz_ast_visitor::visit(astRETURN *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    node->expression->accept(this);
}

void graphviz_ast_visitor::visit(astIF *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->if_block->getLabel() << "\"" << std::endl;
    if(node->else_block != nullptr){ outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->else_block->getLabel() << "\"" << std::endl;}

    node->expression->accept(this);
    node->if_block->accept(this);
    if(node->else_block != nullptr){ node->else_block->accept(this);}
}

void graphviz_ast_visitor::visit(astFOR *node){
    if(node->var_decl != nullptr){ outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->var_decl->getLabel() << "\"" << std::endl;}
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    if(node->assignment != nullptr){ outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->assignment->getLabel() << "\"" << std::endl;}
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->for_block->getLabel() << "\"" << std::endl;

    if(node->var_decl != nullptr){ node->var_decl->accept(this);}
    node->expression->accept(this);
    if(node->assignment != nullptr){ node->assignment->accept(this);}
    node->for_block->accept(this);
}

void graphviz_ast_visitor::visit(astWHILE *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->while_block->getLabel() << "\"" << std::endl;
    
    node->expression->accept(this);
    node->while_block->accept(this);
}

void graphviz_ast_visitor::visit(astFPARAMS *node){
    for(auto &c : *node->children){
        outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << c->getLabel() << "\"" << std::endl;
    }

    for(auto &c : *node->children){
        c->accept(this);
    }
}

void graphviz_ast_visitor::visit(astFPARAM *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->type->getLabel() << "\"" << std::endl;
    
    node->identifier->accept(this);
    node->type->accept(this);
}

void graphviz_ast_visitor::visit(astFUNC_DECL *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->type->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->fparams->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->function_block->getLabel() << "\"" << std::endl;
    
    node->type->accept(this);
    node->identifier->accept(this);
    node->fparams->accept(this);
    node->function_block->accept(this);
}

void graphviz_ast_visitor::visit(astBLOCK *node){
    for(auto &c : *node->children){
        outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << c->getLabel() << "\"" << std::endl;
    }
    
    for(auto &c : *node->children){
        c->accept(this);
    }
}

void graphviz_ast_visitor::visit(astPROGRAM *node){
    for(auto &c : *node->children){
        outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << c->getLabel() << "\"" << std::endl;
    }

    for(auto &c : *node->children){
        c->accept(this);
    }

    outfile << "}" << std::endl;
    outfile.close();
}