#include "pmparser.h"
/**
 * gobal variables
 */

void mapparser::pmparser_parse(int pid){
	char maps_path[500];
	if(pid>=0 ){
		sprintf(maps_path,"/proc/%d/maps",pid);
	}else{
		sprintf(maps_path,"/proc/self/maps");
	}
	FILE* file=fopen(maps_path,"r");
	if(!file){
		fprintf(stderr,"pmparser : cannot open the memory maps, %s\n",strerror(errno));
		return;
	}
	int ind=0;char buf[3000];
	char c;
	procmaps_struct* list_maps=NULL;
	procmaps_struct* tmp;
	procmaps_struct* current_node=list_maps;
	char addr1[20],addr2[20], perm[8], offset[20], dev[10],inode[30],pathname[600];
	int i = 0;
	while(1){
		if( (c=fgetc(file))==EOF ) break;
		fgets(buf+1,259,file);
		buf[0]=c;
		//fill the node
		_pmparser_split_line(buf,addr1,addr2,perm,offset, dev,inode,pathname);
		if (perm[0] != 'r' ||  perm[1] != 'w' || perm[2] == 'x' || perm[3] != 'p' || strcmp(pathname, "[stack]")==0)
			continue;

		//allocate a node
		tmp=&map[ind];
		
		//printf("#%s",buf);
		//printf("%s-%s %s %s %s %s\t%s\n",addr1,addr2,perm,offset,dev,inode,pathname);
		//addr_start & addr_end
		unsigned long l_addr_start;
		sscanf(addr1,"%lx",(long unsigned *)&tmp->addr_start );
		sscanf(addr2,"%lx",(long unsigned *)&tmp->addr_end );
		//size
		tmp->length=(unsigned long)((unsigned long)tmp->addr_end-(unsigned long)tmp->addr_start);
		//perm
		strcpy(tmp->perm,perm);
		tmp->is_r=(perm[0]=='r');
		tmp->is_w=(perm[1]=='w');
		tmp->is_x=(perm[2]=='x');
		tmp->is_p=(perm[3]=='p');

		//offset
		sscanf(offset,"%lx",&tmp->offset );
		//device
		strcpy(tmp->dev,dev);
		//inode
		tmp->inode=atoi(inode);
		//pathname
		strcpy(tmp->pathname,pathname);
		tmp->next=NULL;
		
		
		//attach the node
		ind++;
		//printf("%s",buf);
	}
	length = ind;
}


void mapparser::_pmparser_split_line(
		char*buf,char*addr1,char*addr2,
		char*perm,char* offset,char* device,char*inode,
		char* pathname){
	//
	int orig=0;
	int i=0;
	//addr1
	while(buf[i]!='-'){
		addr1[i-orig]=buf[i];
		i++;
	}
	addr1[i]='\0';
	i++;
	//addr2
	orig=i;
	while(buf[i]!='\t' && buf[i]!=' '){
		addr2[i-orig]=buf[i];
		i++;
	}
	addr2[i-orig]='\0';

	//perm
	while(buf[i]=='\t' || buf[i]==' ')
		i++;
	orig=i;
	while(buf[i]!='\t' && buf[i]!=' '){
		perm[i-orig]=buf[i];
		i++;
	}
	perm[i-orig]='\0';
	//offset
	while(buf[i]=='\t' || buf[i]==' ')
		i++;
	orig=i;
	while(buf[i]!='\t' && buf[i]!=' '){
		offset[i-orig]=buf[i];
		i++;
	}
	offset[i-orig]='\0';
	//dev
	while(buf[i]=='\t' || buf[i]==' ')
		i++;
	orig=i;
	while(buf[i]!='\t' && buf[i]!=' '){
		device[i-orig]=buf[i];
		i++;
	}
	device[i-orig]='\0';
	//inode
	while(buf[i]=='\t' || buf[i]==' ')
		i++;
	orig=i;
	while(buf[i]!='\t' && buf[i]!=' '){
		inode[i-orig]=buf[i];
		i++;
	}
	inode[i-orig]='\0';
	//pathname
	pathname[0]='\0';
	while(buf[i]=='\t' || buf[i]==' ')
		i++;
	orig=i;
	while(buf[i]!='\t' && buf[i]!=' ' && buf[i]!='\n'){
		pathname[i-orig]=buf[i];
		i++;
	}
	pathname[i-orig]='\0';

}

void mapparser::pmparser_print(int index, int order){
	
	procmaps_struct* tmp=&map[index];
	int id=0;
	if(order<0) order=-1;
	while(tmp!=NULL){
		//(unsigned long) tmp->addr_start;
		if(order==id || order==-1){
			printf("Backed by:\t%s\n",strlen(tmp->pathname)==0?"[anonym*]":tmp->pathname);
			printf("Range:\t\t%p-%p\n",tmp->addr_start,tmp->addr_end);
			printf("Length:\t\t%ld\n",tmp->length);
			printf("Offset:\t\t%ld\n",tmp->offset);
			//printf("Permissions:\t%s\n",tmp->perm);
			//printf("Inode:\t\t%d\n",tmp->inode);
			//printf("Device:\t\t%s\n",tmp->dev);
		}
		if(order!=-1 && id>order)
			tmp=NULL;
		else if(order==-1){
			printf("#################################\n");
			tmp=tmp->next;
		}else tmp=tmp->next;

		id++;
	}
}

