// Magsud Rashidli 230AHC024
// Compile: gcc -std=c17 -O2 -Wall -o calc calc.c
#define NAME "Magsud"
#define LASTNAME "Rashidli"
#define ID "230AHC024"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <direct.h>
#include <sys/stat.h>

char *text_buf;
long buf_len = 0;

long err_pos = 0;

void skip_space() {
    while(pos<buf_len){
        char c=text_buf[pos];
        if(c==' '||c=='\t'||c=='\r'||c=='\n'){ pos++; continue; }
        if(c=='#'){ while(pos<buf_len && text_buf[pos]!='\n') pos++; continue; }
        break;
    }
}

double read_number() {
    skip_space();
    char tmp[64]; int i=0,dot=0;
    long start = pos;
    while(pos<buf_len && (isdigit(text_buf[pos])||text_buf[pos]=='.')) {
        if(text_buf[pos]=='.'){ if(dot){ err_pos=start+1; break; } dot=1; }
        if(i<63) tmp[i++]=text_buf[pos];
        pos++;
    }
    tmp[i]='\0';
    if(i==0){ err_pos=start+1; return 0; }
    return atof(tmp);
}

double parse_factor();

double parse_term() {
    double val=parse_factor();
    while(1){
        skip_space();
        if(pos>=buf_len) break;
        char op = text_buf[pos];
        if(op!='*' && op!='/') break;
        pos++;
        double next=parse_factor();
        if(op=='*') val*=next;
        else { if(next==0){ err_pos=pos; break; } val/=next; }
    }
    return val;
}

double parse_expression() {
    double val=parse_term();
    while(1){
        skip_space();
        if(pos>=buf_len) break;
        char op=text_buf[pos];
        if(op!='+' && op!='-') break;
        pos++;
        double next=parse_term();
        if(op=='+') val+=next;
        else val-=next;
    }
    return val;
}

double parse_factor() {
    skip_space();
    if(pos>=buf_len){ err_pos=pos+1; return 0; }
    if(text_buf[pos]=='('){
        pos++;
        double val=parse_expression();
        skip_space();
        if(pos<buf_len && text_buf[pos]==')') pos++;
        else err_pos=pos+1;
        return val;
    } else if(text_buf[pos]=='+'||text_buf[pos]=='-'){
        int sign=(text_buf[pos]=='-')?-1:1;
        pos++;
        return sign*parse_factor();
    } else return read_number();
}

void make_folder(const char *f){
#ifdef _WIN32
    _mkdir(f);
#else
    mkdir(f,0775);
#endif
}

void get_base(const char *p,char *out){
    const char *b=p;
    for(int i=0;p[i];i++) if(p[i]=='/'||p[i]=='\\') b=p+i+1;
    int i=0; while(b[i] && b[i]!='.') { out[i]=b[i]; i++; } out[i]='\0';
}

void process_file(const char *fname,const char *outdir){
    FILE *f=fopen(fname,"r"); if(!f) return;
    fseek(f,0,SEEK_END); buf_len=ftell(f); fseek(f,0,SEEK_SET);
    text_buf=malloc(buf_len+1); fread(text_buf,1,buf_len,f); fclose(f);
    text_buf[buf_len]='\0';
    pos=0; err_pos=0;
    double val=parse_expression();
    skip_space(); if(pos<buf_len && err_pos==0) err_pos=pos+1;
    char base[128]; get_base(fname,base);
    char outpath[1024]; snprintf(outpath,sizeof(outpath),"%s/%s_%s_%s_%s.txt",outdir,base,NAME,LASTNAME,ID);
    FILE *fo=fopen(outpath,"w"); if(!fo){ free(text_buf); return; }
    if(err_pos>0) fprintf(fo,"ERROR:%ld\n",err_pos);
    else fprintf(fo,"%.15g\n",val);
    fclose(fo); free(text_buf);
}

void process_dir(const char *dir,const char *outdir){
    DIR *d=opendir(dir); if(!d) return;
    struct dirent *e; char path[1024];
    while((e=readdir(d))){
        if(e->d_name[0]=='.') continue;
        int l=strlen(e->d_name);
        if(l<4) continue;
        if(e->d_name[l-4]!='.'||e->d_name[l-3]!='t'||e->d_name[l-2]!='x'||e->d_name[l-1]!='t') continue;
        snprintf(path,sizeof(path),"%s/%s",dir,e->d_name);
        process_file(path,outdir);
    }
    closedir(d);
}

int main(int argc,char **argv){
    char *indir=NULL,*outdir=NULL,*file=NULL;
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"-d")==0||strcmp(argv[i],"--dir")==0){ if(i+1<argc) indir=argv[++i]; }
        else if(strcmp(argv[i],"-o")==0||strcmp(argv[i],"--output-dir")==0){ if(i+1<argc) outdir=argv[++i]; }
        else file=argv[i];
    }
    if(!file){ printf("Usage: ./calc input.txt [-d dir] [-o outdir]\n"); return 1; }
    char folder[1024];
    if(outdir) snprintf(folder,sizeof(folder),"%s",outdir);
    else{ char base[64]; get_base(file,base); snprintf(folder,sizeof(folder),"%s_%s_%s_%s",base,NAME,LASTNAME,ID);}
    make_folder(folder);
    if(indir) process_dir(indir,folder);
    else process_file(file,folder);
    return 0;
}

