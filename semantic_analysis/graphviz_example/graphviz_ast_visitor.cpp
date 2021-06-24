//
// Created by Xandru Mifsud on 29/03/2021.
//

#include "graphviz_ast_visitor.h"

graphviz_ast_visitor::graphviz_ast_visitor(string filename){
    outfile.open(filename + ".dot");
    outfile << "digraph ast {" << std::endl;
}

void graphviz_ast_visitor::visit(astTYPE *node){}
void graphviz_ast_visitor::visit(astLITERAL *node){}
void graphviz_ast_visitor::visit(astIDENTIFIER *node){}

void graphviz_ast_visitor::visit(astELEMENT *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->index->getLabel() << "\"" << std::endl;

    node->identifier->accept(this);
    node->index->accept(this);
}

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

void graphviz_ast_visitor::visit(astASSIGNMENT_IDENTIFIER *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    
    node->identifier->accept(this);
    node->expression->accept(this);
}

void graphviz_ast_visitor::visit(astASSIGNMENT_ELEMENT *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->element->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;

    node->element->accept(this);
    node->expression->accept(this);
}

void graphviz_ast_visitor::visit(astASSIGNMENT_MEMBER *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->tls_name->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->assignment->getLabel() << "\"" << std::endl;

    node->tls_name->accept(this);
    node->assignment->accept(this);
}

void graphviz_ast_visitor::visit(astVAR_DECL *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->type->getLabel() << "\"" << std::endl;
    if(node->expression != nullptr){ outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;}
    
    node->identifier->accept(this);
    node->type->accept(this);
    if(node->expression != nullptr){node->expression->accept(this);}
}

void graphviz_ast_visitor::visit(astARR_DECL *node){
    for(auto &c : *node->children){
        outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << c->getLabel() << "\"" << std::endl;
    }

    for(auto &c : *node->children){
        c->accept(this);
    }
}

void graphviz_ast_visitor::visit(astTLS_DECL *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->identifier->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->tls_block->getLabel() << "\"" << std::endl;

    node->identifier->accept(this);
    node->tls_block->accept(this);
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
    if(node->decl != nullptr){ outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->decl->getLabel() << "\"" << std::endl;}
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->expression->getLabel() << "\"" << std::endl;
    if(node->assignment != nullptr){ outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->assignment->getLabel() << "\"" << std::endl;}
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->for_block->getLabel() << "\"" << std::endl;

    if(node->decl != nullptr){ node->decl->accept(this);}
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
    if(node->fparams != nullptr){
        outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->fparams->getLabel() << "\"" << std::endl;
    }
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->function_block->getLabel() << "\"" << std::endl;
    
    node->type->accept(this);
    node->identifier->accept(this);
    if(node->fparams != nullptr){ node->fparams->accept(this);}
    node->function_block->accept(this);
}

void graphviz_ast_visitor::visit(astMEMBER_ACCESS *node){
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->tls_name->getLabel() << "\"" << std::endl;
    outfile << "\"" << node->getLabel() << "\"" << "->" << "\"" << node->member->getLabel() << "\"" << std::endl;

    node->tls_name->accept(this);
    node->member->accept(this);
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