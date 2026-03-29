#include <stdio.h>
#include <assert.h>

#include "GK_MainType.h"
#include "GK_AllFunc.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


// ====================================================================
// USED GLOBAL CONSTANTS
// ====================================================================

extern const char *GK_STD_GRAPH_DUMP;
extern const char *GK_STD_IMG_DUMP;


// ====================================================================
// DECLARATION HELPER FUNCTIONS
// ====================================================================

static void gk_create_block(GK_TreeObject *obj, FILE *stream);
static void gk_create_line(GK_TreeObject *obj, FILE *stream);

// ====================================================================
// MAIN DUMP FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
void GK_TreeDump(GK_Tree *tree) {
    assert(tree);

    GK_CreateGraph(tree->null, GK_STD_GRAPH_DUMP, GK_STD_IMG_DUMP);

    return ;
}

// ------------------------------------------------------------------
void GK_CreateGraph(GK_TreeObject *null, const char *name_dot, const char *name_img) {
    assert(null);
    assert(name_dot);
    assert(name_img);


    FILE *stream = fopen(name_dot, "wb");
    if (stream == NULL)     ExitF("NULL File", );

    // printf("aslabflasbflsblf\n");
    fprintf(stream,
            "digraph {\n"
            "  rankdir=UD;\n"
            "  bgcolor=\"#1e1e1e\"\n"
            "  splines=ortho;\n"
            "  nodesep=0.4;\n"
            "  ranksep=0.6;\n"
            "  node [shape=plaintext, style=filled, fontname=\"Helvetica\"];\n"
            "  edge [arrowhead=vee, arrowsize=0.6, penwidth=1.2];\n\n");


    gk_create_block(null, stream);
    gk_create_line(null, stream);

    fprintf(stream, "\n}\n");
    fclose(stream);

    char comand[200] = {};
    sprintf (comand,
             "dot %s -T png -o %s",
             name_dot, name_img);

    int trash = system (comand);
    (void) trash;

    return ;
}


// ====================================================================
// DUMP HELPER FUNCTIONS
// ====================================================================

// ------------------------------------------------------------------
static void gk_create_block(GK_TreeObject *obj, FILE *stream) {
    assert(obj);
    assert(stream);

    char color[20] = "#2fff24d6";
    char yes_line[20] = "";
    char no_line[20] = "";

    bool is_leaf = true;

    if (obj->branch.no) {
        is_leaf = false;
        gk_create_block(obj->branch.no, stream);
        sprintf(no_line, "%p", obj->branch.no);
    }
    if (obj->branch.yes) {
        is_leaf = false;
        gk_create_block(obj->branch.yes, stream);
        sprintf(yes_line, "%p", obj->branch.yes);
    }

    if (is_leaf) {
        sprintf(color, "#2a18f3");
    }

    fprintf(stream,
            "block_%p [label=<\n<TABLE CELLSPACING=\"0\" CELLPADDING=\"4\">\n"
            "<TR><TD PORT=\"root\" BGCOLOR=\"#0308f9ff\" COLSPAN=\"2\"><B>%p</B></TD></TR>\n"
            "<TR><TD BGCOLOR=\"#f46b8bff\" COLSPAN=\"2\">%s</TD></TR>\n",
            obj, obj, obj->text);

    if (obj->set & GK_TREE_OBJECT_HAVE_IMAGE) {
        fprintf(stream,
                "<TR><TD BGCOLOR=\"#d013ff\">IMG</TD><TD BGCOLOR=\"#ffee30\">%s</TD></TR>\n",
                obj->files.img);
    }
    if (obj->set & GK_TREE_OBJECT_HAVE_SAMPLE) {
        fprintf(stream,
                "<TR><TD BGCOLOR=\"#d013ff\">SAMPLE</TD><TD BGCOLOR=\"#ffee30\">%s</TD></TR>\n",
                obj->files.smp);
    }

    if (obj->kind == GK_TREE_OBJECT_LEAF) {
        fprintf(stream,
                "<TR><TD BGCOLOR=\"#2ffcdd\" COLSPAN=\"2\">LEAF</TD></TR>\n");
    } else {
       fprintf(stream,
                "<TR><TD BGCOLOR=\"#2ffcdd\" COLSPAN=\"2\">BRANCH</TD></TR>\n");
    }

    fprintf(stream,
            "<TR><TD BGCOLOR=\"#ff7301ff\">NO</TD><TD BGCOLOR=\"#08ff3aff\">YES</TD></TR>\n"
            "<TR>\n<TD PORT=\"no\" BGCOLOR=\"#ff7301ff\">%s</TD>\n"
            "<TD PORT=\"yes\" BGCOLOR=\"#08ff3aff\">%s</TD>\n</TR>\n</TABLE> >];\n\n",
            no_line, yes_line);

    return ;
}

// ------------------------------------------------------------------
static void gk_create_line(GK_TreeObject *obj, FILE *stream) {
    assert(obj);
    assert(stream);

    if (obj->branch.no) {
        gk_create_line(obj->branch.no, stream);
        fprintf(stream,
                "block_%p:no -> block_%p:root [color=\"#2bf349\", penwidth = 1.5, arrowsize = 0.6, constraint = true];\n",
                obj, obj->branch.no);
    }
    if (obj->branch.yes) {
        gk_create_line(obj->branch.yes, stream);
        fprintf(stream,
                "block_%p:yes -> block_%p:root [color=\"#2bf349\", penwidth = 1.5, arrowsize = 0.6, constraint = true];\n",
                obj, obj->branch.yes);
    }

    return ;
}


