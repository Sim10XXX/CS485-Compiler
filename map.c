#include "map.h"
//#include "typecheck.h"

void map_insert_sorted(Map* map, char* id, int numchild, int linenum, void* d){
    int i;
    Map temp1;
    Map temp2;
    temp1.id = id;
    temp1.numchild = numchild;
    temp1.linenum = linenum;
    temp1.maptype = map->maptype;
    temp1.data = d;
    for (i = 0; ; i++){
        if (!map[i].id){
            memcpy(&map[i], &temp1, sizeof(Map));
            break;
        }
        if (strcmp(map[i].id, temp1.id) == 0){
            throw_error(temp1.linenum, "Class redefinition");
        }

        else if (strcmp(map[i].id, temp1.id) > 0){
            memcpy(&temp2, &map[i], sizeof(Map));
            memcpy(&map[i], &temp1, sizeof(Map));
            memcpy(&temp1, &temp2, sizeof(Map));
        }
    }
}

void map_insert_ast(Map* map, AST* class){
    int i, j;
    int numc = 0;
    Attribute* attributes;
    ImpMethod* methods;
    char* parent;
    if (map->maptype == parent_map){
        if (class->children[0]->nodetype == Inherits){
            parent = class->children[0]->id;
        }
        else if (class->children[0]->nodetype == No_inherits){
            parent = "Object";
        }
        map_insert_sorted(map, class->id, 1, class->linenum, parent);
        return;
    }

    for (i = 0 ; i < class->numchild ; i++){
        if (map->maptype == attribute_map && (class->children[i]->nodetype == Attribute_no_init || class->children[i]->nodetype == Attribute_init)){
            numc++;
        }
        else if (map->maptype == method_map && class->children[i]->nodetype == Method) {
            numc++;
        }
    }
    if (map->maptype == attribute_map){
        attributes = malloc(sizeof(Attribute)*numc);
    }
    else if (map->maptype == method_map){
        methods = malloc(sizeof(ImpMethod)*numc);
    }
    
    numc = 0;
    for (i = 0 ; i < class->numchild ; i++){
        if (map->maptype == attribute_map && (class->children[i]->nodetype == Attribute_no_init || class->children[i]->nodetype == Attribute_init)){
            attributes[numc].id = class->children[i]->id;
            attributes[numc].type = class->children[i]->children[0]->id;
            attributes[numc].init = class->children[i]->nodetype;
            attributes[numc].linenum = class->children[i]->linenum;

            if (class->children[i]->nodetype == Attribute_init){
                attributes[numc].expr = class->children[i]->children[1];
            }
            else{
                attributes[numc].expr = NULL;
            }

            numc++;
        }
        else if (map->maptype == method_map && class->children[i]->nodetype == Method){
            methods[numc].class = class->id;
            methods[numc].id = class->children[i]->id;
            methods[numc].linenum = class->children[i]->linenum;
            methods[numc].numformals = class->children[i]->numchild - 2;
            methods[numc].formals = malloc(sizeof(AST*) * methods[numc].numformals);
            for (j = 0; j < methods[numc].numformals; j++){
                methods[numc].formals[j] = class->children[i]->children[j];
            }
            methods[numc].returntype = class->children[i]->children[j]->id;
            methods[numc].expr = class->children[i]->children[j+1];

            numc++;
        }
    }
    if (map->maptype == attribute_map){
        map_insert_sorted(map, class->id, numc, class->linenum, attributes);
    }
    else if (map->maptype == method_map){
        map_insert_sorted(map, class->id, numc, class->linenum, methods);
    }
    
}

//this adds the 5 built-in classes
Map* map_initialize(int numchild, MapType maptype){
    Map* map;
    int i, j;
    if (maptype == parent_map){
        map = malloc(sizeof(Map)*(numchild + 5));
        memset(map, 0, sizeof(Map)*(numchild + 5));
        map[0].id = "Bool";
        map[1].id = "IO";
        map[2].id = "Int";
        map[3].id = "String";
        for (i = 0; i < numchild + 4; i++){
            map[i].maptype = maptype;
            map[i].data = "Object";
        }
        return map;
    }
    //else
    map = malloc(sizeof(Map)*(numchild + 6));
    memset(map, 0, sizeof(Map)*(numchild + 6));
    for (i = 0; i < numchild + 5; i++){
        map[i].maptype = maptype;
    }
    if (maptype == implementation_map) return map;
    if (maptype == class_map) return map;
    map[0].id = "Bool";
    map[1].id = "IO";
    map[2].id = "Int";
    map[3].id = "Object";
    map[4].id = "String";

    //define the predefined methods
    if (maptype == method_map){
        ImpMethod* mdata;
        AST* temptype, *tempformal;
        //IO methods
        mdata = malloc(sizeof(ImpMethod) * 4);
        memset(mdata, 0, sizeof(ImpMethod) * 4);
        //in_int() : Int
        mdata[0].id = "in_int";
        mdata[0].class = "IO";
        mdata[0].returntype = "Int";
        //in_string() : String
        mdata[1].id = "in_string";
        mdata[1].class = "IO";
        mdata[1].returntype = "String";
        //out_int(x : Int) : SELF_TYPE
        mdata[2].id = "out_int";
        mdata[2].class = "IO";
        mdata[2].returntype = "SELF_TYPE";
        mdata[2].numformals = 1;
        temptype = malloc(sizeof(AST));
        memset(temptype, 0, sizeof(AST));
        temptype->nodetype = Type;
        temptype->id = "Int";
        tempformal = malloc(sizeof(AST));
        memset(tempformal, 0, sizeof(AST));
        tempformal->nodetype = Formal;
        tempformal->id = "x";
        tempformal->numchild = 1;
        tempformal->children = malloc(sizeof(AST*));
        tempformal->children[0] = temptype;
        mdata[2].formals = malloc(sizeof(AST*));
        mdata[2].formals[0] = tempformal;
        //out_string(x : String) : SELF_TYPE
        mdata[3].id = "out_string";
        mdata[3].class = "IO";
        mdata[3].returntype = "SELF_TYPE";
        mdata[3].numformals = 1;
        temptype = malloc(sizeof(AST));
        memset(temptype, 0, sizeof(AST));
        temptype->nodetype = Type;
        temptype->id = "String";
        tempformal = malloc(sizeof(AST));
        memset(tempformal, 0, sizeof(AST));
        tempformal->nodetype = Formal;
        tempformal->id = "x";
        tempformal->numchild = 1;
        tempformal->children = malloc(sizeof(AST*));
        tempformal->children[0] = temptype;
        mdata[3].formals = malloc(sizeof(AST*));
        mdata[3].formals[0] = tempformal;

        //insert base methods into IO's method list
        map[1].data = mdata;
        map[1].numchild = 4;

        //Object methods
        mdata = malloc(sizeof(ImpMethod) * 3);
        memset(mdata, 0, sizeof(ImpMethod) * 3);
        //abort() : Object
        mdata[0].id = "abort";
        mdata[0].class = "Object";
        mdata[0].returntype = "Object";
        //copy() : SELF_TYPE
        mdata[1].id = "copy";
        mdata[1].class = "Object";
        mdata[1].returntype = "SELF_TYPE";
        //type_name() : String
        mdata[2].id = "type_name";
        mdata[2].class = "Object";
        mdata[2].returntype = "String";

        //insert base methods into Objects's method list
        map[3].data = mdata;
        map[3].numchild = 3;

        //String methods
        mdata = malloc(sizeof(ImpMethod) * 3);
        memset(mdata, 0, sizeof(ImpMethod) * 3);
        //concat(s : String) : String
        mdata[0].id = "concat";
        mdata[0].class = "String";
        mdata[0].returntype = "String";
        mdata[0].numformals = 1;
        temptype = malloc(sizeof(AST));
        memset(temptype, 0, sizeof(AST));
        temptype->nodetype = Type;
        temptype->id = "String";
        tempformal = malloc(sizeof(AST));
        memset(tempformal, 0, sizeof(AST));
        tempformal->nodetype = Formal;
        tempformal->id = "s";
        tempformal->numchild = 1;
        tempformal->children = malloc(sizeof(AST*));
        tempformal->children[0] = temptype;
        mdata[0].formals = malloc(sizeof(AST*));
        mdata[0].formals[0] = tempformal;
        //length() : Int
        mdata[1].id = "length";
        mdata[1].class = "String";
        mdata[1].returntype = "Int";
        //substr(i : Int, l : Int) : String
        mdata[2].id = "substr";
        mdata[2].class = "String";
        mdata[2].returntype = "String";
        mdata[2].numformals = 2;
        tempformal = malloc(sizeof(AST)*2);
        memset(tempformal, 0, sizeof(AST)*2);
        tempformal[0].nodetype = Formal;
        tempformal[0].id = "i";
        tempformal[0].numchild = 1;
        tempformal[0].children = malloc(sizeof(AST*));
        temptype = malloc(sizeof(AST));
        memset(temptype, 0, sizeof(AST));
        temptype->nodetype = Type;
        temptype->id = "Int";
        tempformal[0].children[0] = temptype;

        tempformal[1].nodetype = Formal;
        tempformal[1].id = "l";
        tempformal[1].numchild = 1;
        tempformal[1].children = malloc(sizeof(AST*));
        temptype = malloc(sizeof(AST));
        memset(temptype, 0, sizeof(AST));
        temptype->nodetype = Type;
        temptype->id = "Int";
        tempformal[1].children[0] = temptype;
        mdata[2].formals = malloc(sizeof(AST*)*2);
        mdata[2].formals[0] = &tempformal[0];
        mdata[2].formals[1] = &tempformal[1];
        //insert base methods into String's method list
        map[4].data = mdata;
        map[4].numchild = 3;

        //define the method bodies
        for (i = 0; i < 5; i++){
            for (j = 0; j < map[i].numchild; j++){
                mdata = (ImpMethod*) map[i].data;
                mdata[j].expr = malloc(sizeof(AST));
                memset(mdata[j].expr, 0, sizeof(AST));
                mdata[j].expr->nodetype = Internal;
                mdata[j].expr->outtype = mdata[j].returntype;
                mdata[j].expr->id = malloc(sizeof(char)*128);
                sprintf(mdata[j].expr->id, "%s.%s", mdata[j].class, mdata[j].id);
            }
        }
        
    }
    return map;
}

void map_attribute_print(Attribute* att, int num, FILE* outf){
    if (!att) return;
    int i;
    for (i = 0; i < num; i++){
        if (att[i].init == Attribute_no_init){
            fprintf(outf, "no_initializer\n");
        }
        else if (att[i].init == Attribute_init){
            fprintf(outf, "initializer\n");
        }
        fprintf(outf, "%s\n", att[i].id);
        fprintf(outf, "%s\n", att[i].type);
        if (att[i].init == Attribute_init){
            ast_print(att[i].expr);
        }
    }
    
}

void map_method_print(ImpMethod* met, int num, FILE* outf){
    if (!met) return;
    int i, j;
    for (i = 0; i < num; i++){
        fprintf(outf, "%s\n", met[i].id);
        fprintf(outf, "%i\n", met[i].numformals);
        for (j = 0; j < met[i].numformals; j++){
            fprintf(outf, "%s\n", met[i].formals[j]->id);
        }
        fprintf(outf, "%s\n", met[i].class);
        ast_print(met[i].expr);
    }
}

void map_print(Map* map, FILE* outf){
    int i;
    int numc;
    switch(map->maptype){
    case class_map:
        fprintf(outf, "class_map\n");
        break;
    case implementation_map:
        fprintf(outf, "implementation_map\n");
        break;
    case parent_map:
        fprintf(outf, "parent_map\n");
        break;
    }


    numc = 0;
    for (i = 0; map[i].id != NULL; i++){
        numc++;
    }
    fprintf(outf, "%i\n", numc);
    
    for (i = 0; map[i].id != NULL; i++){
        fprintf(outf, "%s\n", map[i].id);
        if (map->maptype == parent_map){
            fprintf(outf, "%s\n", (char*)map[i].data);
            continue;
        }
        fprintf(outf, "%i\n", map[i].numchild);
        switch(map->maptype){
        case class_map:
            map_attribute_print((Attribute*) map[i].data, map[i].numchild, outf);
            break;
        case implementation_map:
            map_method_print((ImpMethod*) map[i].data, map[i].numchild, outf);
            break;

        }
        
    }
}

Map* map_get_map_by_id(Map* map, char* id){
    int i;
    if (!map) printf("null map\n");
    if (!id) printf("null id\n");
    //printf("b\n");
    for (i = 0; map[i].id != NULL; i++){
        //printf("b: %i\n", i);
        if (strcmp(map[i].id, id) == 0){
            return &map[i];
        }
    }
    return NULL;
}

void* map_get_data_from_map_by_id(Map* map, char* id){
    if (!map) return NULL;
    int i;
    Attribute* att;
    ImpMethod* met;
    if (map->maptype == class_map){
        att = (Attribute*) map->data;
        for (i = 0; i < map->numchild; i++){
            if (strcmp(id, att[i].id) == 0){
                return &att[i];
            }
        }
    }
    else if (map->maptype == method_map || map->maptype == implementation_map){
        met = (ImpMethod*) map->data;
        for (i = 0; i < map->numchild; i++){
            if (strcmp(id, met[i].id) == 0){
                return &met[i];
            }
        }
    }
    return NULL;
}

int map_len(Map* map){
    int i;
    for (i = 0; map[i].id != NULL; i++){
    }
    return i;
}

Map* map_read_parent_map(FILE* fptr){
    char buffer[BUFMAX];
    fgets(buffer, BUFMAX, fptr);
    int n = atoi(buffer) + 1;
    Map* pmap = malloc(sizeof(Map) * n);
    memset(pmap, 0, sizeof(Map) * n);
    int i;
    for (i = 0; i < n-1; i++){
        pmap[i].maptype = parent_map;
        pmap[i].numchild = 1;
        pmap[i].id = read_id(fptr);
        pmap[i].data = read_id(fptr);
    }
    return pmap;
}
Map* map_read_implementation_map(FILE* fptr){
    char buffer[BUFMAX];
    fgets(buffer, BUFMAX, fptr);
    int n = atoi(buffer) + 1;
    Map* imap = malloc(sizeof(Map) * n);
    memset(imap, 0, sizeof(Map) * n);
    int i, j, k;
    ImpMethod* mdata;
    AST** tempformals;
    for (i = 0; i < n-1; i++){
        imap[i].maptype = implementation_map;
        imap[i].id = read_id(fptr);
        fgets(buffer, BUFMAX, fptr);
        imap[i].numchild = atoi(buffer)+1; //+1 to add .new method
        
        mdata = malloc(sizeof(ImpMethod) * imap[i].numchild);
        memset(mdata, 0, sizeof(ImpMethod) * imap[i].numchild);
        imap[i].data = mdata;
        for (j = 0; j < imap[i].numchild; j++){
            if (j == 0){
                mdata[j].id = ".new";
                mdata[j].class = imap[i].id;
                mdata[j].returntype = imap[i].id;
                mdata[j].expr = malloc(sizeof(AST));
                memset(mdata[j].expr, 0, sizeof(AST));
                mdata[j].expr->nodetype = Internal;
                continue;
            }
            mdata[j].id = read_id(fptr);
            fgets(buffer, BUFMAX, fptr);
            mdata[j].numformals = atoi(buffer);

            tempformals = malloc(sizeof(AST*)*mdata[j].numformals);
            memset(tempformals, 0, sizeof(AST*)*mdata[j].numformals);
            for (k = 0; k < mdata[j].numformals; k++){
                tempformals[k] = malloc(sizeof(AST));
                memset(tempformals[k], 0, sizeof(AST));
                tempformals[k]->nodetype = Formal;
                tempformals[k]->id = read_id(fptr);
                //tempformal[k].numchild = 0;
            }

            mdata[j].formals = tempformals;

            mdata[j].class = read_id(fptr);
            fgets(buffer, BUFMAX, fptr);
            mdata[j].linenum = atoi(buffer);
            mdata[j].returntype = read_id(fptr);
            mdata[j].expr = malloc(sizeof(AST));
            fgets(buffer, BUFMAX, fptr);
            //printf("arr1\n");
            ast_recursive_read(mdata[j].expr, fptr, ast_type_from_buffer(buffer));
            //printf("arr2\n");
        }
    }
    return imap;
}

Map* map_read_class_map(FILE* fptr){
    char buffer[BUFMAX];
    fgets(buffer, BUFMAX, fptr);
    int n = atoi(buffer);
    Map* cmap = malloc(sizeof(Map) * n);
    memset(cmap, 0, sizeof(Map) * n);
    Attribute* adata;
    int i, j;
    for (i = 0; i < n; i++){
        //printf("c: %i\n", i);
        cmap[i].maptype = class_map;
        cmap[i].id = read_id(fptr);
        //printf("id: %s\n", cmap[i].id);
        fgets(buffer, BUFMAX, fptr);
        cmap[i].numchild = atoi(buffer);
        //printf("num: %i\n", cmap[i].numchild);
        if (cmap[i].numchild > 0){
            adata = malloc(sizeof(Attribute) * cmap[i].numchild);
            memset(adata, 0, sizeof(Attribute) * cmap[i].numchild);
            cmap[i].data = adata;
        }
        
        for (j = 0; j < cmap[i].numchild; j++){
            fgets(buffer, BUFMAX, fptr);
            if (strcmp("initializer\n", buffer) == 0){
                adata[j].init = Attribute_init;
                //printf("init\n");
            }
            else{
                adata[j].init = Attribute_no_init;
                //printf("no_init\n");
            }
            adata[j].id = read_id(fptr);
            //printf("att name: %s\n", adata[j].id);
            adata[j].type = read_id(fptr);
            //printf("att type: %s\n", adata[j].type);
            //printf("%i, %i\n",adata[j].init, Attribute_init);
            if (adata[j].init == Attribute_init){
                //fgets(buffer, BUFMAX, fptr);
                //printf("read expr\n");
                adata[j].expr = malloc(sizeof(AST));
                ast_recursive_read(adata[j].expr, fptr, Expr);
                //printf("line: %i\n", adata[j].expr->linenum);
            }
            
        }
    }
    return cmap;
}

int map_get_offset(Map* map, char* class, char* id){ //naive implementation, doesn't work for dynamic dispatch
    if (class != NULL){
        map = map_get_map_by_id(map, class);
    }
    int i, j;
    ImpMethod* mdata;
    Attribute* adata;
    if (map->maptype == implementation_map){
        for (i = 0; map[i].id != NULL; i++){
            mdata = (ImpMethod*) map[i].data;
            for (j = 0; j < map[i].numchild; j++){
                if (strcmp(mdata[j].id, id) == 0){
                    return (j+1)*8; //j + 1 because the vtable in assembly contains a typename as the first element
                }
            }
        }
        
    }
    else if (map->maptype == class_map){
        adata = (Attribute*) map->data;
        for (i = 0; i < map->numchild; i++){
            if (strcmp(adata[i].id, id) == 0){
                return (i+3)*8; // + 3 because class tag, object size, vtable
            }
            
        }
    }
    
    return -1;
}

int map_get_param_offset(Map* impmap, char* currmethod, char* id){
    if (!impmap || !currmethod || !id) return -1;
    int i;
    ImpMethod* mdata;

    if (strcmp("self", id) == 0){
        //printf("self param used\n");
        return 2*8;
    }
    mdata = (ImpMethod*) map_get_data_from_map_by_id(impmap, currmethod);
    for (i = 0; i < mdata->numformals; i++){
        if (strcmp(mdata->formals[i]->id, id) == 0){
            return ((mdata->numformals-1)-i +3 )*8; //i+3 to skip over rbp and self object (and smth else idk), -i because of how params are pushed
        }
    }

    
    return -1;
}

char* map_get_type(Map* impmap, Map* classmap, char* currmethod, char* id){
    if (!impmap || !currmethod || !id) return NULL;
    int i;
    ImpMethod* mdata;

    if (strcmp("self", id) == 0){
        //printf("self param used\n");
        return impmap->id;
    }
    mdata = (ImpMethod*) map_get_data_from_map_by_id(impmap, currmethod);
    for (i = 0; i < mdata->numformals; i++){
        if (strcmp(mdata->formals[i]->id, id) == 0){
            //return mdata->formals[i]->;
        }
    }

    
    return NULL;
}

//returns index in alist of the nearest ancestor in alist to id. returns -1 if no ancestor found
int map_nearest_ancestor(Map* pmap, char** alist, char* id){
    int n, i;
    for (n = 0; alist[n]!= NULL; n++){}
    
    Map* m;
    while (1){
        for (i = 0; i < n; i++){
            if (strcmp(alist[i], id) == 0){
                return i;
            }
        }
        m = map_get_map_by_id(pmap, id);
        if (m == NULL){
            return -1;
        }
        id = m->data;
    }
    
    return -1;
}

List* map_get_all_subclasses(Map* pmap, char* class){
    //printf("%i\n", map_len(pmap));
    if (!pmap || !class){
        printf("Null param\n");
        return NULL;
    }
    List* classes = list_new();
    List* subclasses;
    list_add_id(classes, class);

    int i;
    //printf("currclass: %s\n", class);
    for (i = 0; pmap[i].id != NULL; i++){
        if (strcmp(pmap[i].data, class) == 0){
            subclasses = map_get_all_subclasses(pmap, pmap[i].id);
            list_append(classes, subclasses);
            list_free(subclasses);
        }
    }
    //printf("s5\n");
    //printf("__\n");
    //for (i = 0; classes->data[i] != NULL; i++){
    //    printf("%s\n", classes->data[i]);
    //}
    //printf("--\n");
    return classes;
}