#include "emit.h"
#include <cstdlib>
#include <cstring>

size_t ptr_count = 0;
size_t char_count = 0;
size_t string_count = 0;
size_t int_count = 0;
size_t ind_ptr_count = 0;
symbol_table* symtable = new symbol_table;
void emit_node(FILE* file, astree* node);
void emit_expr(FILE* file, astree* node);
void emit_new_vreg(FILE* file, astree* node);

void indent(FILE* file){
   fprintf(file, "        ");
}
string new_vreg(char type){
   string builder;
   switch(type){
      case 'p':
         builder += type;
         builder += to_string(++ptr_count);
         break;
      case 'c':
         builder += type;
         builder += to_string(++char_count);
         break;
      case 'i':
         builder += type; 
         builder += to_string(++int_count);
         break;
      case 'a':
         builder += type; 
         builder += to_string(++ind_ptr_count);
         break;
      case 's':
         builder += type;
         builder += to_string(++string_count);
         break;
      default:
      errprintf("Invalid register type: %c \n", type);
   }
   return builder;
}

void emit_while(FILE* file, astree* node){
   fprintf(file, "while_%zu_%zu_%zu:;\n",
      node->filenr, node->linenr, node->offset);
   switch(node->children[0]->symbol){
      case TOK_IDENT:
      case TOK_CHARCON:
      case TOK_INTCON:
         break;
      default:
         indent(file);
         emit_expr(file, node->children[0]);
   }
   indent(file);
   fprintf(file,
      "if (!%s) goto break_%zu_%zu_%zu\n", 
      node->children[0]->vreg.c_str(), 
      node->children[1]->filenr,
      node->children[1]->linenr,
      node->children[1]->offset);
   emit_node(file, node->children[1]);
   indent(file);
   fprintf(file,
      "goto while_%zu_%zu_%zu\n", 
      node->filenr, 
      node->linenr, 
      node->offset);
   fprintf(file,
      "break_%zu_%zu_%zu):;\n",
      node->children[1]->filenr,
      node->children[1]->linenr,
      node->children[1]->offset);
}

void emit_if(FILE* file, astree* node){
   switch(node->children[0]->symbol){
      case TOK_IDENT:
      case TOK_CHARCON:
      case TOK_INTCON:
         break;
      default:
         indent(file);
         emit_expr(file, node->children[0]);
   }
      indent(file);
      fprintf(file,
         "if (!%s) goto fi_%zu_%zu_%zu;\n", 
         node->children[0]->vreg.c_str(), 
         node->children[0]->filenr,
         node->children[0]->linenr,
         node->children[0]->offset);
      for(auto child : node->children[0]->children){
         indent(file);
         emit_node(file, child);
      }
      fprintf(file,
         "fi_%zu_%zu_%zu:;\n",
         node->children[0]->filenr,
         node->children[0]->linenr,
         node->children[0]->offset);

}

void emit_ifelse(FILE* file, astree* node){
   switch(node->children[0]->symbol){
      case TOK_IDENT:
      case TOK_CHARCON:
      case TOK_INTCON:
         break;
      default:
         indent(file);
         emit_expr(file, node->children[0]);
   }
      indent(file);
      fprintf(file,
         "if (!%s) goto else_%zu_%zu_%zu;\n", 
         node->children[0]->vreg.c_str(), 
         node->children[0]->filenr,
         node->children[0]->linenr,
         node->children[0]->offset);
      for(auto child : node->children[0]->children){
         indent(file);
         emit_node(file, child);
      }
      fprintf(file,
         "goto fi_%zu_%zu_%zu:;\n",
         node->children[0]->filenr,
         node->children[0]->linenr,
         node->children[0]->offset);
      fprintf(file,
         "else_%zu_%zu_%zu:;\n",
         node->children[0]->filenr,
         node->children[0]->linenr,
         node->children[0]->offset);
      for(auto child : node->children[1]->children){
         indent(file);
         emit_node(file, child);
      }
      fprintf(file,
         "fi_%zu_%zu_%zu:;\n",
         node->children[0]->filenr,
         node->children[0]->linenr,
         node->children[0]->offset);

}

//void emit_expr(FILE* file, astree* node){
//   symbol* sym = nullptr;
//   char* type_buf = nullptr;
//   int bval = 0;
//   switch(node->symbol){
//      case TOK_FALSE:
//      case TOK_TRUE:
//         node->vreg = new_vreg('c');
//         type_buf = strdup("char");
//         if(node->symbol == TOK_TRUE)
//            bval = 1;
//         fprintf(file, "%s %s = %d;\n", 
//            type_buf,
//            node->vreg.c_str(), bval);
//         st_insert(symtable, node);
//         break;
//      case TOK_IDENT:
//         sym = st_lookup(symtable, node);
//         if(sym != nullptr){
//            fprintf(file, " %s ", 
//               node->vreg.c_str());
//            break;
//         }
//         //fall thru to decl fprintf if undecl
//      case TOK_INTCON:
//      case TOK_CHARCON:
//         if(node->attr[attr_bool] || node->attr[attr_char]){
//            node->vreg = new_vreg('c');
//            type_buf = strdup("char");
//         }
//         else if(node->attr[attr_int]){
//            node->vreg = new_vreg('i');
//            type_buf = strdup("int");
//         }
//         else if(node->attr[attr_struct]){
//            node->vreg = new_vreg('p');
//            type_buf = (char*) node->children[0]->lexinfo->c_str();
//         }
//         else{
//            fprintf(file, "%s;\n", node->lexinfo->c_str());
//            break;
//         }
//         fprintf(file, "%s %s = %s;\n", 
//            type_buf,
//            node->vreg.c_str(), node->lexinfo->c_str());
//         st_insert(symtable, node);
//         break;
//      case '+':
//      case '-':
//      case '/':
//      case '*':
//      case '%':
//      case TOK_EQ:
//      case TOK_NE:
//      case TOK_LT:
//      case TOK_LE:
//      case TOK_GT:
//      case TOK_GE:
//         if(node->attr[attr_bool] || node->attr[attr_char]){
//            node->vreg = new_vreg('c');
//            type_buf = strdup("char");
//         }
//         else if(node->attr[attr_int]){
//            node->vreg = new_vreg('i');
//            type_buf = strdup("int");
//         }
//         else if(node->attr[attr_struct]){
//            node->vreg = new_vreg('p');
//            type_buf = (char*) node->children[0]->lexinfo->c_str();
//         }
//         //for 90-c8q.oc
//         if(node->children.empty() || 
//               node->children[0] == nullptr || 
//               node->children[1] == nullptr)
//            break;
//         fprintf(file, "%s %s = %s %s %s;\n", 
//            type_buf,
//            node->vreg.c_str(),
//            node->children[0]->vreg.c_str(),
//            node->lexinfo->c_str(),
//            node->children[1]->vreg.c_str()); 
//         break;
//      case TOK_ORD:
//         node->vreg = new_vreg('i');
//         type_buf = strdup("int");
//         node->vreg = new_vreg('c');
//         type_buf = strdup("char");
//         fprintf(file, "%s %s = %s;\n", 
//            type_buf,
//            node->vreg.c_str(), node->lexinfo->c_str());
//         st_insert(symtable, node);
//         break;
//      case TOK_CHR:
//         node->vreg = new_vreg('c');
//         type_buf = strdup("char");
//         fprintf(file, "%s %s = %s;\n", 
//            type_buf,
//            node->vreg.c_str(), node->lexinfo->c_str());
//         st_insert(symtable, node);
//         break;
//      case TOK_NEG:
//         node->vreg = new_vreg('i');
//         type_buf = strdup("int");
//         fprintf(file, "%s %s = %s;\n", 
//            type_buf,
//            node->vreg.c_str(), node->lexinfo->c_str());
//         st_insert(symtable, node);
//         break;
//         
//         default:
//            errprintf("unkown expression: %s", node->lexinfo->c_str());
//   }
//}

void emit_binop(FILE* file, astree* node){
   //emits in the form: a * b
   //if it exist, don't redecl
   symbol* sym = st_lookup(symtable, node);
   string vreg;
   if(sym != nullptr){
      vreg = sym->vreg;
      fprintf(file, "%s %s %s", 
            node->children[0]->vreg.c_str(),
            node->lexinfo->c_str(),
            node->children[1]->vreg.c_str()
            );
   }
   else{
      //emit_new_vreg(file, node);
      fprintf(file, "%s %s %s",
            node->children[0]->vreg.c_str(),
            node->lexinfo->c_str(),
            node->children[1]->vreg.c_str()
            );
   }
}

void emit_ident(FILE* file, astree* node){
   printf("Emit ident: %s\n", node->lexinfo->c_str());
   string ident = mangle_ident(node);
   fprintf(file, ident.c_str());
   node->vreg = ident;
}

void emit_expr(FILE* file, astree* node){
   if(node->attr[attr_vreg]){
      emit_binop(file, node);
   }
   else{
      switch(node->symbol){
         case TOK_IDENT:
            emit_ident(file, node);
            break;
         case TOK_INTCON:
         case TOK_CHARCON:
            emit_new_vreg(file, node);
            break;
      }
   }
   fprintf(file, "\n");
}

void emit_rec(FILE* file, astree* node){
   for(auto child : node->children)
      emit_rec(file, child);
   emit_node(file, node);

}

void emit_return(FILE* file, astree* node){
   if(node->children.empty()){
      indent(file);
      fprintf(file, "return;\n");
   }
   else{
      fprintf(file, "return %s;\n", node->children[0]->vreg.c_str());
   }
}

void emit_assignment(FILE* file, astree* node){
   symbol* sym = st_lookup(symtable, node);
   //if sym is null declare it
   char* type_buf = nullptr;
   indent(file);
   if(node->attr[attr_bool] || node->attr[attr_char]){
      node->vreg = new_vreg('c');
      type_buf = strdup("char");
   }
   else if(node->attr[attr_int]){
      node->vreg = new_vreg('i');
      type_buf = strdup("int");
   }
   else if(node->attr[attr_struct]){
      node->vreg = new_vreg('p');
      type_buf = (char*) node->children[0]->lexinfo->c_str();
   }
   if(sym == nullptr){
      fprintf(file, "%s %s %s %s;\n",
            type_buf, node->children[0]->vreg.c_str(),
            node->lexinfo->c_str(), node->vreg.c_str());
   }
   else{
      fprintf(file, "%s %s %s;\n",
            node->children[0]->vreg.c_str(),
            node->lexinfo->c_str(),
            node->vreg.c_str());
   }
}

void emit_new_vreg(FILE* file, astree* node){
   //emits in the form: int i4
   char* type_buf;
   if(node->attr[attr_bool] || node->attr[attr_char]){
      node->vreg = new_vreg('c');
      type_buf = strdup("char");
   }
   else if(node->attr[attr_int]){
      node->vreg = new_vreg('i');
      type_buf = strdup("int");
   }
   else if(node->attr[attr_struct]){
      node->vreg = new_vreg('p');
      type_buf = (char*) node->children[0]->lexinfo->c_str();
   }
   else{
      node->vreg = new_vreg('a');
      type_buf = strdup("unknown");
   }
   fprintf(file, "%s %s", 
      type_buf,
      node->vreg.c_str());
   st_insert(symtable, node);

}

void emit_call(FILE* file, astree* node){
   //emits in the form i1 = int func(param, param);
   if(!node->attr[attr_void]){
      //return type & assignment
      emit_new_vreg(file, node);
   }
      fprintf(file, "%s ( ", 
            node->children.back()->lexinfo->c_str());
      //print params
      for(auto child = node->children.begin(); 
            child != node->children.end()-1; child++){
            fprintf(file, "%s", (*child)->vreg.c_str());
            if(child != node->children.end()-2)
               fprintf(file, ", ");
            else
               fprintf(file, " );\n");
      }

}

void emit_node(FILE* file, astree* node){
   switch(node->symbol){
      case TOK_WHILE:
         emit_while(file, node);
         break;
      case TOK_IF:
         emit_if(file, node);
         break;
      case TOK_IFELSE:
         emit_ifelse(file, node);
         break;
      case TOK_VARDECL:
         emit_assignment(file, node);
         break;
      case TOK_RETURN:
         emit_return(file, node); 
         break; 
      case TOK_CALL:
         emit_call(file, node);
         break;


      case '+':
      case '-':
      case '*':
      case '%':
      case TOK_EQ:
      case TOK_NE:
      case TOK_LE:
      case TOK_GE:
      case TOK_LT:
      case TOK_GT:
      case TOK_POS:
      case TOK_NEG:
      case TOK_INTCON:
      case TOK_IDENT:
      case TOK_CHARCON:
         emit_expr(file, node);
         break; 
   }
}

void emit_stringcon(FILE* file, astree* node){
   string vreg = new_vreg('s');
   node->vreg = vreg;
   fprintf(file, "char* %s = %s;\n", 
      vreg.c_str(), node->lexinfo->c_str());
   st_insert(symtable, node);

}

string mangle_struct(astree* node){
   //node is structdecl
   string builder;
   builder += "struct s_" + *node->children[0]->lexinfo + "{" + "\n";
   for(auto field = node->children.cbegin()+1; 
         field != node->children.cend(); field++){
      builder += "        ";
      builder += *(*field)->lexinfo;
      builder += " _f";
      builder += *(*field)->children[0]->lexinfo;
      builder += "_";
      builder += *(*field)->children[0]->lexinfo;
      builder += ";\n";
   }
   builder += "};\n";
   return builder;
}

string mangle_ident(astree* node){
   string builder = *node->lexinfo + " ";
   astree* child = nullptr;
   if (!node->children.empty())
      child = node->children[0];
   else 
      child = node;
   //global
   if (node->blocknr == 0){
      builder += "__";
      builder += *child->lexinfo;
   }
   //local
   else{
      builder += "_";
      builder += child->blocknr;
      builder +="_";
      builder += *child->lexinfo;
   }
   builder += ";\n";
   return builder;
}

string mangle_param(astree* node){
   string builder = *node->lexinfo + " ";
   astree* child = node->children[0];
   builder += "_";
   builder += to_string(child->blocknr);
   builder +="_";
   builder += *child->lexinfo;
   return builder;
}
void emit_func(FILE* file, astree* node){
   //symtable.enter_block();
   string builder = "__" + *node->children[0]->lexinfo + " ";
   builder += *node->children[0]->children[0]->lexinfo + " (\n";
   for(auto param : node->children[1]->children){
      builder += "        " + mangle_param(param) + ",\n";
   }
   //replace last comma with paren
   builder.pop_back();
   builder.pop_back();
   builder += ")\n{\n";
   fprintf(file, builder.c_str());
   for(auto func_block : node->children[2]->children){
      emit_rec(file, func_block);
   }
   fprintf(file, "}\n");
   //symtable.leave_block();
}

void emit_oil(FILE* file, astree* root){
   //change all bool to char
   //first structs
   for(auto child : root->children){
      if(child->symbol == TOK_STRUCT){
         fprintf(file, mangle_struct(child).c_str());
      }
   }
   //ocmain header
   fprintf(file, "void __ocmain (void) {\n");
   //next stringcon
   for(auto node : stringcon_list){
      indent(file);
      emit_stringcon(file, node);
   }
   //global vars
   for(auto child : root->children){
      if(child->symbol == TOK_VARDECL){
         fprintf(file, "        ");
         child->vreg = mangle_ident(child->children[0]);
         fprintf(file, child->vreg.c_str());
      }
      else if (child->symbol != TOK_FUNCTION && child->symbol != TOK_PROTOTYPE){
         //rest of expr and stmt in global scope
         emit_rec(file, root);
      }
   }
   fprintf(file, "}\n");
   //funcs
   for(auto child : root->children){
      if(child->symbol == TOK_FUNCTION){
         emit_func(file, child);
      }
   }
}

