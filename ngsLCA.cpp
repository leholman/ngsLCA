/*
  1649555 -> 1333996
  1401172 -> 1333996
  1582271 -> 1582270

 */

int mod_in[] =  {1649555 , 1401172 ,1582271, 374764, 242716,1793725 ,292451};
int mod_out[]=  {1333996 , 1333996 ,1582270,1914213,1917265,1915309, 263865 };

#include <cassert>
#include <cstdio>
#include <zlib.h>
#include <cstring>
#include <map>
#include <cstdlib>
#include <htslib/hts.h>
#include <htslib/sam.h>
#include <vector>

struct cmp_str
{
   bool operator()(char const *a, char const *b)
   {
      return std::strcmp(a, b) < 0;
   }
};

typedef std::map<char *, int, cmp_str> char2int;
typedef std::map<int,char *> int2char;
typedef std::map<int,int> int2int;

void mod_db(int *in,int *out,int2int &parent, int2char &rank,int2char &name_map){
  for(int i=0;i<7;i++){
    assert(parent.count(out[i])==1);
    parent[in[i]] = parent[out[i]];
    rank[in[i]] = rank[out[i]];
    name_map[in[i]] = strdup("satan");
  }

}
int2int errmap;

//usefull little function to split
char *strpop(char **str,char split){
  char *tok=*str;
  while(**str){
    if(**str!=split)
      (*str)++;
    else{
      **str='\0'; (*str)++;
      break;
    }
  }
  return tok;
}
//usefull little function to remove tab and newlines
void strip(char *line){
  int at=0;
  //  fprintf(stderr,"%s\n",line);
  for(int i=0;i<strlen(line);i++)
    if(line[i]=='\t'||line[i]=='\n')
      continue;
    else
      line[at++]=line[i];
  //  fprintf(stderr,"at:%d line:%p\n",at,line);
  line[at]='\0';
  //fprintf(stderr,"%s\n",line);
}




int2int ref2tax(const char *fname,bam_hdr_t *hdr ){
  char2int revmap;
  for(int i=0;i<hdr->n_targets;i++)
    revmap[hdr->target_name[i]] = i;
  fprintf(stderr,"\t-> Number of SQ tags:%d from bamfile: \'%s\'\n",revmap.size());


  
  int2int am;
  gzFile gz= Z_NULL;
  gz=gzopen(fname,"rb");
  if(gz==Z_NULL){
    fprintf(stderr,"\t-> Problems opening file: \'%s\'\n",fname);
    exit(0);
  }
  char buf[4096];
  int at=0;
  while(gzgets(gz,buf,4096)){
    if(!((at++ %100000 ) ))
       fprintf(stderr,"\r\t-> At linenr: %d in \'%s\'      ",at,fname);
    strtok(buf,"\t\n ");
    char *key =strtok(NULL,"\t\n ");
    int val = atoi(strtok(NULL,"\t\n "));

    //check if the key exists in the bamheader, if not then skip this taxid
    
    char2int::iterator it=revmap.find(key);
    if(it==revmap.end())
      continue;

    if(am.find(it->second)!=am.end())
      fprintf(stderr,"\t-> Duplicate entries found \'%s\'\n",key);
    else
      am[it->second] = val;

  }
  fprintf(stderr,"\n");
  fprintf(stderr,"\t-> [%s] Number of entries to use from accesion to taxid: %lu\n",fname,am.size());
  return am;
}

int nodes2root(int taxa,int2int &parent){
  int dist =0;
  while(taxa!=1){
    int2int::iterator it = parent.find(taxa);
    taxa = it->second;
    dist++;
  }
  return dist;
}



int do_lca(std::vector<int> &taxids,int2int &parent){
  //  fprintf(stderr,"\t-> [%s] with number of taxids: %lu\n",__func__,taxids.size());
  assert(taxids.size()>0);
  if(taxids.size()==1){
    int taxa=taxids[0];
    if(parent.count(taxa)==0){
      fprintf(stderr,"\t-> Problem finding taxaid: %d will skip\n",taxa);
      taxids.clear();
      return -1;
    }


    return taxa;
  }

  int2int counter;
  for(int i=0;i<taxids.size();i++){
    int taxa = taxids[i];
    while(1){
      //      fprintf(stderr,"taxa:%d\n",taxa);
      int2int::iterator it=counter.find(taxa);
      if(it==counter.end()){
	//	fprintf(stderr,"taxa: %d is new will plugin\n",taxa);
	counter[taxa]=1;
      }else
	it->second = it->second+1;
      it = parent.find(taxa);

      if(it==parent.end()){
	int2int::iterator it=errmap.find(taxa);
	if(it==errmap.end()){
	  fprintf(stderr,"\t-> Problem finding parent of :%d\n",taxa);
	  
	  errmap[taxa] = 1;
	}else
	  it->second = it->second +1;
	taxids.clear();
	return -1;
      }
      
      if(taxa==it->second)//<- catch root
	break;
      taxa=it->second;
    }

  }
  //  fprintf(stderr,"counter.size():%lu\n",counter.size());
  //now counts contain how many time a node is traversed to the root
  int2int dist2root;
  for(int2int::iterator it=counter.begin();it!=counter.end();it++)
    if(it->second==taxids.size())
      dist2root[nodes2root(it->first,parent)] = it->first;
  for(int2int::iterator it=dist2root.begin();0&&it!=dist2root.end();it++)
    fprintf(stderr,"%d\t->%d\n",it->first,it->second);
  taxids.clear();
  if(!dist2root.empty())
    return (--dist2root.end())->second;
}

void print_chain1(FILE *fp,int taxa,int2char &rank,int2char &name_map){
  int2char::iterator it1=name_map.find(taxa);
  int2char::iterator it2=rank.find(taxa);
  if(it1==name_map.end()){
    fprintf(stderr,"\t-> Problem finding taxaid:%d\n",taxa);
  }
  assert(it2!=rank.end());
  if(it1==name_map.end()||it2==rank.end()){
    fprintf(stderr,"taxa: %d %s doesnt exists will exit\n",taxa,it1->second);
    exit(0);
  }
  fprintf(fp,"\t%d:%s:%s",taxa,it1->second,it2->second);
  
}


void print_chain(FILE *fp,int taxa,int2int &parent,int2char &rank,int2char &name_map){

    while(1){
      print_chain1(fp,taxa,rank,name_map);
      int2int::iterator it = parent.find(taxa);
      assert(it!=parent.end());
      if(taxa==it->second)//<- catch root
	break;
      taxa=it->second;
    }
    fprintf(fp,"\n");
}

void hts(const char *fname,int2int &i2i,int2int& parent,bam_hdr_t *hdr,int2char &rank, int2char &name_map){
  samFile *fp_in = hts_open(fname,"r"); //open bam file
  bam1_t *aln = bam_init1(); //initialize an alignment
  bam_hdr_t *bamHdr = sam_hdr_read(fp_in)  ;
  int comp ;

  char *last=NULL;
  std::vector<int> taxids;
  int lca;
  while(sam_read1(fp_in,bamHdr,aln) > 0){
    char *qname = bam_get_qname(aln);
    int chr = aln->core.tid ; //contig name (chromosome)

    if(last==NULL)
      last=strdup(qname);
    if(strcmp(last,qname)!=0){
      if(taxids.size()>0){
	int size=taxids.size();
	lca=do_lca(taxids,parent);
	if(lca!=-1){
	  fprintf(stdout,"%s:%lu\t",last,size);fflush(stdout);
	  print_chain(stdout,lca,parent,rank,name_map);
	}
      }
      free(last);
      last=strdup(qname);
    }
    
    int2int::iterator it = i2i.find(chr);
    //filter by nm
    uint8_t *nm = bam_aux_get(aln,"NM");
    if(nm!=NULL){
      int val = (int) bam_aux2i(nm);
      // fprintf(stderr,"nm:%d\n",val);
      if(val>0){
	continue;
	fprintf(stderr,"skip: %s\n",last);
      }
    }


    
    if(it==i2i.end())
      fprintf(stderr,"\t-> problem finding chrid:%d chrname:%s\n",chr,hdr->target_name[chr]);
    else
      taxids.push_back(it->second);
  }
  if(taxids.size()>0){
    int size=taxids.size();
    if(lca!=-1){
      lca=do_lca(taxids,parent);
      if(lca!=-1){
	fprintf(stdout,"%s:%lu",last,size);fflush(stdout);
	print_chain(stdout,lca,parent,rank,name_map);
      }
    }
  }
  bam_destroy1(aln);
  sam_close(fp_in);
  
  return ;//0;
}

int2char parse_names(const char *fname){

  gzFile gz= Z_NULL;
  gz=gzopen(fname,"rb");
  if(gz==Z_NULL){
    fprintf(stderr,"\t-> Problems opening file: \'%s\'\n",fname);
    exit(0);
  }
  int2char name_map;
  char buf[4096];
  int at=0;
  char **toks = new char*[5];
  
  while(gzgets(gz,buf,4096)){
    strip(buf);//fprintf(stderr,"buf:%s\n",buf);
    char *saveptr = buf;
    toks[0]=strpop(&saveptr,'|');
    toks[1]= strpop(&saveptr,'|');
    toks[2]= strpop(&saveptr,'|');
    toks[3]= strpop(&saveptr,'|');
    for(int i=0;0&&i<4;i++)
      fprintf(stderr,"%d):\'%s\'\n",i,toks[i]);

    int key=atoi(toks[0]);
    //    fprintf(stderr,"key:%d\n",key);
    if(toks[3]&&strcmp(toks[3],"scientific name")==0){
      int2char::iterator it=name_map.find(key);
      
      if(it!=name_map.end())
	fprintf(stderr,"\t->[%s] duplicate name(column1): %s\n",fname,toks[0]);
      else
	name_map[key]=strdup(toks[1]);

    }
    if(0&&at++>10)
      break;
  }
  //  int2char::iterator it = name_map.find(61564);  assert(it!=name_map.end());
  fprintf(stderr,"\t-> [%s] Number of unique names (column1): %lu with third column 'scientific name'\n",fname,name_map.size());
  return name_map;
}


void parse_nodes(const char *fname,int2char &rank,int2int &parent){

  gzFile gz= Z_NULL;
  gz=gzopen(fname,"rb");
  if(gz==Z_NULL){
    fprintf(stderr,"\t-> Problems opening file: \'%s\'\n",fname);
    exit(0);
  }
  char buf[4096];
  int at=0;
  char **toks = new char*[5];

  while(gzgets(gz,buf,4096)){
    strip(buf);//fprintf(stderr,"buf:%s\n",buf);
    char *saveptr = buf;
    toks[0]= strpop(&saveptr,'|');
    toks[1]= strpop(&saveptr,'|');
    toks[2]= strpop(&saveptr,'|');
    for(int i=0;0&&i<3;i++)
      fprintf(stderr,"%d):\'%s\'\n",i,toks[i]);

    int2int::iterator it = parent.find(atoi(toks[0]));
    if(it!=parent.end())
      fprintf(stderr,"\t->[%s] duplicate name(column0): %s\n",fname,toks[0]);
    else{
      int key=atoi(toks[0]);
      int val= atoi(toks[1]);
      parent[key]=val;
      rank[key]=strdup(toks[2]);
    }
  }
  fprintf(stderr,"\t-> Number of unique names (column1): %lu from file: %s\n",rank.size(),fname);

}


int main(int argc, char **argv){
  const char *htsfile="CHL_155_12485.sort.bam";
  //const char *htsfile="asdf.bam";
  const char *as2tax="nucl_gb.accession2taxid.gz";
  const char *nodes = "nodes.dmp.gz";
  const char *names = "names.dmp.gz";
  const char *fasta = "tmp.gz";
  fprintf(stderr,"\t-> as2fax:%s nodes:%d hts:%s names:%s fasta:%s\n\n",as2tax,nodes,htsfile,names,fasta);
  
  samFile *fp_in = hts_open(htsfile,"r"); //open bam file
  bam_hdr_t *bamHdr = sam_hdr_read(fp_in);
  sam_close(fp_in);

  //map of taxid -> taxid
  int2int parent;
  //map of taxid -> rank
  int2char rank;
  
  //map of bamref ->taxid
  int2int i2i= ref2tax(as2tax,bamHdr);
  parse_nodes(nodes,rank,parent);
  //map of taxid -> name
  int2char name_map = parse_names(names);
  int2char::iterator it = name_map.find(61564);
  assert(it!=name_map.end());
  fprintf(stderr,"\t-> Will add some fixes of the ncbi database due to merged names\n");
  mod_db(mod_in,mod_out,parent,rank,name_map);
  
  hts(htsfile,i2i,parent,bamHdr,rank,name_map);
  for(int2int::iterator it=errmap.begin();it!=errmap.end();it++)
    fprintf(stderr,"err\t%d\t%d\n",it->first,it->second);
  return 0;
}