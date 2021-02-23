#include <stdio.h>
#include <stdlib.h>


int mem_size = 16384; 
int logical_size = 65536;
int page_size =  256;
int frame_size = 256;
int p_tablesize =  512;
int ffptr=2;
int t_frames = 0;
int t_pages = 0;
#define BACKINGSTORE "BACKING_STORE_1.bin"


unsigned int parseChar(unsigned char c){
	
	if(c>='0' && c<='9')return c-'0';
	else if(c>='a' && c<='f')return 10+c-'a'; 	  //help taken from https://stackoverflow.com/questions/3408706/hexadecimal-string-to-byte-array-in-c
	else if(c>='A' && c<='F')return 10+c-'A';
		
}
unsigned int parseString(char * c,int size){
	unsigned int hexValue=0;
	for(int i=0;i<size;i++){
		hexValue<<=4;
		hexValue+=parseChar(c[i]);
	}
	return hexValue;
}

int getPageNumber(int address){
	address>>=8;				
	return address & 0xFF;
}
int getOffset(int address){
	return address & 0xFF;		
}
unsigned char getUp8(int address){
	address>>=8;			
	return address & 0xFF;
}
unsigned char getLow8(int val){ 
	return (char)val & 0xFF;		
}
int checkInMemory(unsigned char val){				
	return 1 & val;
}	
int checkDirty(unsigned char val){				
	return 2& val;	
}
int checkUsed(unsigned char val){
	return 4 & val;
}
void setInMemory(unsigned char * val){
	*val |= 1;
}
void removeInMemory(unsigned char *val){
	unsigned char mask =1;
	mask = ~mask;
	*val &= mask;
}
void setDirty(unsigned char *val){
	*val |= 2;
}
void setUsed(unsigned char * val){
	*val |= 4;
}
void decreaseUsed(unsigned char * val){
	unsigned char mask = 4;
	mask =~mask;
	*val &= mask;
}

void readFrame(unsigned char * memory,int frameNum,int pageIndex){
	int pageNum=pageIndex/2;
	FILE *fptr;
	fptr=fopen("BACKING_STORE_1.bin","ab+");
	unsigned int frameStartingPos=frameNum*page_size;
	fseek(fptr,pageNum*page_size,SEEK_SET);
	fread(memory+frameStartingPos,sizeof(unsigned char),page_size,fptr);
	fclose(fptr);

}
void writeBackToStore(unsigned char * memory, int frameNum,int pageIndex){
	int pageNum=pageIndex/2;	
	FILE *file;
	file=fopen("BACKING_STORE_1.bin","ab+");
	unsigned int frameStartingPos=frameNum*page_size;
	fseek(file,pageNum*page_size,SEEK_SET);
	fwrite(memory+frameStartingPos,sizeof(unsigned char),page_size,file);
	fclose(file);
}
int bringPageIntoMemory(int address,unsigned char* pageTable,int pageIndex){ 
	if(ffptr<t_frames){
		pageTable[pageIndex+1]=ffptr;
		readFrame(pageTable,ffptr,pageIndex);
		pageTable[pageIndex]=0;
		setInMemory(&pageTable[pageIndex]);
		return ffptr++;
	}
	else{
		
		int i, j;
		for(j=0;j<2;j++){
			int secondOption=-1;
			for(i=0;i<p_tablesize;i+=2){
				if(checkInMemory(pageTable[i])){
					if(!checkUsed(pageTable[i]) && !checkDirty(pageTable[i])){
						pageTable[pageIndex+1]=pageTable[i+1];
						removeInMemory(&pageTable[i]);
						readFrame(pageTable,(int)pageTable[i+1],pageIndex);
						pageTable[pageIndex]=0;
						setInMemory(&pageTable[pageIndex]);
						return pageTable[i+1];
					}
					else if (checkUsed(pageTable[i])){
						decreaseUsed(&pageTable[i]);
					}
					else if (secondOption==-1){
						secondOption=i;
					}
				}
			}
			if(secondOption!=-1){
				writeBackToStore(pageTable,pageTable[secondOption+1],pageIndex);
				pageTable[pageIndex+1]=pageTable[secondOption+1];
				removeInMemory(&pageTable[secondOption]);
				readFrame(pageTable,(int)pageTable[secondOption+1],pageIndex);
				pageTable[pageIndex]=0;
				setInMemory(&pageTable[pageIndex]);
				return pageTable[secondOption+1];
			}
		}
	}
	printf("Error Something Went Wrong\n");
	exit(1);
}

int getFrame(int address,unsigned char * pageTable){

	int pageFault;

	if (address>=logical_size || address<0 ){
		printf("Invalid Address Access\n");
		exit(1);
	}
	else{
		int pageIndex=2*getPageNumber(address);
		unsigned char check,frameAddress;
		check = pageTable[pageIndex];

		if(checkInMemory(check)){		
			frameAddress = pageTable[pageIndex+1];
			pageFault=0;		//In memory so return the frame address	
			setUsed(&pageTable[pageIndex]); 
		}
		else{
			frameAddress=bringPageIntoMemory(address,pageTable,pageIndex);
			pageFault=1;
 		}
 		pageFault<<=8;
 		
 		return pageFault | frameAddress;
	}
}
int readFromMemory(int address,unsigned char * pageTable, unsigned char* val){	
	int rawFrame=getFrame(address,pageTable);
	int offset = getOffset(address);
	int frame = getLow8(rawFrame);
	int pageFault = getUp8(rawFrame);
	unsigned char value=pageTable[frame*page_size+offset];
	int physicalAddress = (frame<<8)| offset;
	if(pageFault){
		printf(" 0x%04X      	 0x%04X           %s           0x%04X       Yes\n", address, physicalAddress, val, value);
	}
	else{
		printf(" 0x%04X      	 0x%04X           %s           0x%04X       No\n", address, physicalAddress, val, value);
	}	
	return pageFault;
}
int writeToMemory(int address,unsigned char * pageTable, unsigned char* val){
	int retVal=readFromMemory(address,pageTable, val);
	int pageIndex = 2*getPageNumber(address);
	setDirty(&pageTable[pageIndex]);
	return retVal;
}
int main(int argc, char *argv[]) {
	
	t_frames =  mem_size/frame_size;
	t_pages= logical_size /page_size;
	char* filename = argv[1];

	if(argc < 2){
		printf("Address file is missing");
		return 1;
	}
	unsigned char * mm = (unsigned char*)malloc(sizeof(unsigned char) * mem_size);
	
	int i;
	for (i=0;i<mem_size;i++){
		mm[i]=0;
	}
	
	
	FILE * file;
	file= fopen(argv[1],"r");
	printf("Logical Address    Physical Addr     Read/Write     Value     PageFault\n");
	unsigned char buffer[9];
	unsigned char bit;
	int add[8];
	double counter=0.0;
	int pageFaultCount=0.0;
	while(!feof(file))
	{
	  fscanf(file, "%s", buffer);
	  unsigned int hex = parseString(buffer+2,4);
	  fscanf(file, "%s", &bit);
	  if(bit == '0'){
	  	unsigned int hex = parseString(buffer+2,4);
		unsigned char* Read = "Read \0";
	  	pageFaultCount+=readFromMemory(hex,mm, Read);
	  }
	  else if (bit == '1'){
		unsigned int hex = parseString(buffer+2,4);
	  	unsigned char* Write = "Write\0";  
		pageFaultCount+=writeToMemory(hex,mm, Write);
	  }
	  counter++;
	}
	fclose(file);
	printf("Page Fault Rate = %f\n", (pageFaultCount / counter)*100);
	free(mm);
	return 0;
}

