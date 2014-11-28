
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"

astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = intern_stringset (lexinfo);
   tree->blocknr = 0;
   tree->attr = 0;
   tree->struct_table = nullptr;
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo->c_str());
   return tree;
}

astree* new_function (
      astree* identdecl, astree* paramlist, astree* block){
   if(!string(";").compare(*block->lexinfo))
      return new_proto(identdecl, paramlist);
   astree* func = new_astree(TOK_FUNCTION, 
      identdecl->filenr, 
      identdecl->linenr, 
      identdecl->offset, "");
   func = adopt2(func, identdecl, paramlist); 
   return adopt1(func, block);

}

astree* new_proto (astree* identdecl, astree* paramlist){
   astree* func = new_astree(TOK_PROTOTYPE, 
      identdecl->filenr, 
      identdecl->linenr, 
      identdecl->offset, "");
   return adopt2(func, identdecl, paramlist); 

}


astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

astree* adopt2sym (
      astree* root, astree* left, astree* right, int symbol){
   left = adopt1(root, left);
   right = adopt1(root, right);
   left->symbol = symbol;
   right->symbol = symbol;
   return root;
}

astree* change_sym (astree* root, int symbol){
   root->symbol = symbol;
   return root;
}

static string enum_tostring(size_t i){
   switch(i){
   case 0: return "attr_void";
   case 1: return "attr_bool";
   case 2: return "attr_char";
   case 3: return "attr_int";
   case 4: return "attr_null";
   case 5: return "attr_string";
   case 6: return "attr_struct";
   case 7: return "attr_array";
   case 8: return "attr_function";
   case 9: return "attr_variable";
   case 10: return "attr_field";
   case 11: return "attr_typeid";
   case 12: return "attr_param";
   case 13: return "attr_lval";
   case 14: return "attr_const";
   case 15: return "attr_vreg";
   case 16: return "attr_vaddr";
   case 17: return "attr_bitset_size";
   }
   return "invalid_enum";
}

static string enum_bitset(attr_bitset a){
   string buf;
   for(int i=0; i<attr_bitset_size; ++i){
      if(a[i]){
         buf += enum_tostring(i);
         buf += " ";
      }
   }
   return buf;
}


static void dump_node (FILE* outfile, astree* node) {
      char* tokname = (char*) get_yytname(node->symbol);
      //dont want to inc single chars
      if (strlen(tokname)>3)
         tokname += 4;
      fprintf(outfile, 
         "%s \"%s\" %zu.%zu.%zu {%zu} %s\n",
         tokname,
         (node->lexinfo)->c_str(), 
         node->filenr,
         node->linenr,
         node->offset,
         node->blocknr,
         enum_bitset(node->attr).c_str()
         );
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   //fprintf (outfile, "%*s%s ", depth * 3, "", root->lexinfo->c_str());
   for (int i=0; i<=depth; i++)
           fprintf(outfile, "|\t");
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child) {
      dump_astree_rec (outfile, root->children[child], depth + 1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep) {
   DEBUGF ('f', "toknum = %d, yyvaluep = %p\n", toknum, yyvaluep);
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n", get_yytname (toknum), toknum);
   }
   fflush (NULL);
}


void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%X]-> %d:%d.%d: %s: \"%s\")\n",
           (uintptr_t) root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

RCSC("$Id: astree.cc,v 1.14 2013-10-10 18:48:18-07 - - $")
