#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE 256
#define MAX_VARS 64
#define MAX_FUNCS 16
#define MAX_BODY 64

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_STR } VarType;

typedef struct {
    char name[32];
    VarType type;
    union {
        int i;
        float f;
        char s[128];
    } value;
} Variable;

typedef struct {
    char name[32];
    char argnames[2][32];
    VarType argtypes[2];
    int argcount;
    char body[MAX_BODY][MAX_LINE];
    int bodylines;
} Function;

// Globale variabelen/functies
Variable vars[MAX_VARS]; int vcount = 0;
Function funcs[MAX_FUNCS]; int fcount = 0;

// Variabelen ondersteunen
Variable *find_var(const char *name) {
    for(int i=0;i<vcount;i++) if(strcmp(vars[i].name, name)==0) return &vars[i];
    return NULL;
}
void set_var(const char *name, VarType t, const char *val) {
    Variable *v = find_var(name);
    if(!v) v = &vars[vcount++], strcpy(v->name,name);
    v->type = t;
    if(t==TYPE_INT) v->value.i = atoi(val);
    else if(t==TYPE_FLOAT) v->value.f = atof(val);
    else if(t==TYPE_STR) strncpy(v->value.s, val, 127);
}
void parse_var(char *line) {
    char typ[8], name[32], val[128];
    if(sscanf(line,"var %s : %s = %[^\n;]",typ,name,val)==3) {
        if(strcmp(typ,"int")==0) set_var(name,TYPE_INT,val);
        else if(strcmp(typ,"float")==0) set_var(name,TYPE_FLOAT,val);
        else if(strcmp(typ,"str")==0) {
            char *s = strchr(val,'"'), *e = strrchr(val,'"');
            if(s && e && e>s) { *e=0; set_var(name,TYPE_STR,s+1); }
            else set_var(name,TYPE_STR,val);
        }
    }
}

// Printen
void exec_writeline(char *line) {
    char *p = strchr(line,'('), *q = strrchr(line,')');
    if(!p || !q) return;
    char inner[200]; strncpy(inner,p+1,q-p-1); inner[q-p-1]=0;
    char *plus = strchr(inner,'+');
    if(!plus) {
        char *s = strchr(inner,'"'), *e = strrchr(inner,'"');
        if(s && e && e>s) { *e=0; printf("%s\n",s+1); }
        else printf("%s\n",inner);
    } else {
        char val[200]="", vn[32]="";
        char *s = strchr(inner,'"'), *e = s?strchr(s+1,'"'):NULL;
        if(s && e && e>s) { strncpy(val,s+1,e-s-1); val[e-s-1]=0; }
        sscanf(plus+1," %s",vn);
        Variable *v = find_var(vn);
        if(v) {
            if(v->type==TYPE_INT) printf("%s%d\n",val,v->value.i);
            else if(v->type==TYPE_FLOAT) printf("%s%f\n",val,v->value.f);
            else printf("%s%s\n",val, v->value.s);
        } else printf("%s%s\n",val,vn);
    }
}

// If & else met blokken { ... }
void exec_block(FILE *fin); // forward
void exec_ifelse(char *cond, FILE *fin) {
    char vn[32]; int val;
    if(sscanf(cond,"if %s == %d",vn,&val)!=2) return;
    Variable *v = find_var(vn);
    int where = 0;
    char block1[MAX_BODY][MAX_LINE]; int bl1 = 0;
    char block2[MAX_BODY][MAX_LINE]; int bl2 = 0;
    char line[MAX_LINE];
    int inelse = 0;
    // Lees blok(ken)
    while(fgets(line,MAX_LINE,fin)) {
        if(strchr(line,'{')) continue;
        if(strstr(line,"else")) { inelse=1; continue;}
        if(strchr(line,'}')) break;
        if(inelse) strncpy(block2[bl2++],line,MAX_LINE-1);
        else       strncpy(block1[bl1++],line,MAX_LINE-1);
    }
    // Uitvoeren
    if(v && v->type==TYPE_INT && v->value.i==val) {
        for(int i=0;i<bl1;i++) exec_block((FILE *)fmemopen(block1[i],strlen(block1[i]),"r"));
    } else {
        for(int i=0;i<bl2;i++) exec_block((FILE *)fmemopen(block2[i],strlen(block2[i]),"r"));
    }
}

// Functies
void parse_function(FILE *fin, char *defline) {
    char name[32], args[64];
    if(sscanf(defline,"fcn %[^ (](%[^)]",name,args)>=1) {
        Function *f = &funcs[fcount++];
        strcpy(f->name,name);
        char atype[16], aname[32]; f->argcount=0;
        char *tok=strtok(args,",");
        while(tok && f->argcount<2) {
            sscanf(tok,"var %s : %s",atype,aname);
            f->argtypes[f->argcount] = strcmp(atype,"int")==0?TYPE_INT:strcmp(atype,"float")==0?TYPE_FLOAT:TYPE_STR;
            strcpy(f->argnames[f->argcount++],aname);
            tok=strtok(NULL,",");
        }
        f->bodylines=0;
        char line[MAX_LINE];
        while(fgets(line,MAX_LINE,fin)) {
            if(strchr(line,'}')) break;
            strcpy(f->body[f->bodylines++],line);
        }
    }
}
Function *find_func(const char *name) {
    for(int i=0;i<fcount;i++) if(strcmp(name,funcs[i].name)==0) return &funcs[i];
    return NULL;
}
void exec_funcall(char *line) {
    char name[32], arg1[128]="", arg2[128]="";
    sscanf(line,"fcn : %[^(:](%*s\"%[^\"]\"%*[^0123456789:] : %s",name,arg1,arg2);
    Function *f = find_func(name);
    if(!f) { printf("[!] Function not found: %s\n",name); return; }
    Variable locs[2] = {0}; // simpel local args
    if(f->argcount>0) { strcpy(locs[0].name,f->argnames[0]); locs[0].type=f->argtypes[0]; if(locs[0].type==TYPE_INT) locs[0].value.i=atoi(arg1); else strcpy(locs[0].value.s,arg1);}
    if(f->argcount>1) { strcpy(locs[1].name,f->argnames[1]); locs[1].type=f->argtypes[1]; if(locs[1].type==TYPE_INT) locs[1].value.i=atoi(arg2); else strcpy(locs[1].value.s,arg2);}
    // alleen WriteLines in functiebody voor demo
    for(int i=0;i<f->bodylines;i++) {
        char *pline = f->body[i];
        if(strstr(pline,"WriteLine")) {
            char *plus = strchr(pline,'+');
            if(plus) {
                char label[128]="",vn[32]="";
                char *str = strchr(pline,'"'),*e=str?strchr(str+1,'"'):NULL;
                if(str&&e&&e>str) { strncpy(label,str+1,e-str-1); label[e-str-1]=0; }
                sscanf(plus+1," %s",vn);
                int found = 0;
                for(int a=0;a<f->argcount;a++)
                    if(strcmp(vn,locs[a].name)==0) {
                        if(locs[a].type==TYPE_INT)
                            printf("%s%d\n",label,locs[a].value.i);
                        else
                            printf("%s%s\n",label,locs[a].value.s);
                        found=1;
                    }
                if(!found) printf("%s%s\n",label,vn);
            }
        }
    }
}

// While-loops tot N
void exec_while(FILE *fin) {
    char body[MAX_BODY][MAX_LINE]; int blen=0;
    char until_line[MAX_LINE]={0};
    while(fgets(body[blen],MAX_LINE,fin)) {
        if(strstr(body[blen],"until while")) { strcpy(until_line, body[blen]); break; }
        ++blen;
    }
    int cnt = 1;
    sscanf(until_line,"until while : %d",&cnt);
    for(int i=0;i<cnt;i++) for(int j=0;j<blen;j++) if(strstr(body[j],"WriteLine")) exec_writeline(body[j]);
}

// "line runner" voor blokken uit memory (in if/else/funclocal)
void exec_block(FILE *fin) {
    char line[MAX_LINE];
    while(fgets(line,MAX_LINE,fin)) {
        if(strstr(line,"var ")) parse_var(line);
        else if(strstr(line,"WriteLine")) exec_writeline(line);
    }
}

void interpret(const char *fname) {
    FILE *fin = fopen(fname,"r");
    if(!fin) { perror("open"); exit(1);}
    char line[MAX_LINE];
    while(fgets(line,MAX_LINE,fin)) {
        if(strstr(line,"var ")) parse_var(line);
        else if(strstr(line,"WriteLine")) exec_writeline(line);
        else if(strstr(line,"if ")) exec_ifelse(line,fin);
        else if(strncmp(line,"while true",10)==0) exec_while(fin);
        else if(strncmp(line,"fcn ",4)==0) parse_function(fin,line);
        else if(strncmp(line,"fcn :",5)==0) exec_funcall(line);
    }
    fclose(fin);
}

int main(int argc, char **argv) {
    if(argc<2) { puts("Juse: ./a program.alang"); return 1; }
    interpret(argv[1]);
    return 0;
}
