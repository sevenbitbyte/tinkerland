#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>


#define TRUE 1
#define FALSE 0

struct FolderInfo {
	char *name;
	long double size;
	char *path;
	long double fileCount;
	//bool 
	struct FolderInfo *parent;
	struct FolderInfo *previous;
	struct FolderInfo *next;
};

struct FileInfo {
	char *name;
	long double size;
	char *path;
	unsigned char isLink;
	struct FolderInfo *parent;
	struct FileInfo *previous;
	struct FileInfo *next;
};


/**********************
Function Prototypes
*********************/

void *allocFileStruct();
void *allocFolderStruct();
void *populateFileInfo(struct stat *, struct FileInfo *, char *);
void *populateFolderInfo(struct FolderInfo *, char *, char *);
void *findFolderEnd(struct FolderInfo *);
void *findFileStart(struct FileInfo *);
void *findFileEnd(struct FileInfo *);
void readDirContent(struct dirent *, DIR *, struct FolderInfo *, struct FileInfo *, struct FileInfo *);
void freeFileStruct(struct FileInfo *);
void freeFolderStruct(struct FolderInfo *);
void *orderFiles(struct FileInfo *);
void addToList(struct FileInfo *, struct FileInfo *);
void removeFileFromList(struct FileInfo *);

int main( int argc, char **argv ){
//printf("%i\n", sizeof(unsigned long long double));
//exit(0);
DIR *rootDir, *currentDir;
//struct FolderInfo rootFolder;
struct FolderInfo *currentFolder;
struct FileInfo *unsortedFiles;
struct FileInfo *currentFile;
struct dirent *dirContent;

//printf("U Self: %x\tSelf2: %x\tNext: %x\tNext: %x\n", unsortedFiles, &unsortedFiles, &unsortedFiles->next, unsortedFiles->next);
//printf("C Self: %x\tSelf2: %x\tNext: %x\tNext: %x\n", currentFile, &currentFile, &currentFile->next, currentFile->next);

unsortedFiles=allocFileStruct();
currentFile=unsortedFiles;
unsortedFiles->previous=NULL;
unsortedFiles->next=unsortedFiles;

if(argc != 2){ //Check for required number of arguments
	//print usage info and exit
	return 0;
}

if( !(rootDir=opendir(argv[1])) ){
	printf("Directory '%s' could not be opened!\n", argv[1]);
	return 0; //Exit due to error
}

currentDir=rootDir; //Set the current directory to point to the root directory
currentFolder=allocFolderStruct();
currentFolder->previous=NULL;
currentFolder->next=NULL;
currentFolder->parent=NULL;
currentFolder->path = calloc( strlen(argv[1])+1, sizeof(char) );
strcpy(currentFolder->path, argv[1]);
currentFolder->name = calloc( 5, sizeof(char) );
strcpy(currentFolder->name, "root\0");
readDirContent(dirContent, currentDir, currentFolder, unsortedFiles, currentFile);

printf("Sorting Files, High to Low...");
printf("Complete(%x:%x)\n", orderFiles(currentFile), currentFile);
printf("!!CLEAN UP TIME!! (%x)\n", currentFile);
//sleep(2);
freeFileStruct(currentFile);
freeFolderStruct(currentFolder);
return 0;
}



void readDirContent(struct dirent *dirContent, DIR *currentDir, struct FolderInfo *parentFolder, struct FileInfo *unsortedFiles, struct FileInfo *currentFile){

//currentFolder->size=0;
//currentFolder->fileCount=0;
struct FolderInfo *currentFolder;
currentFolder=parentFolder;
/*if(parentFolder->parent != NULL){
if(parentFolder->parent->next->next != NULL){
	printf("Directory(%s:%s:%s): %s\n", parentFolder->parent->name, parentFolder->parent->next->name, parentFolder->parent->next->next->name, parentFolder->path);
}
else{
	printf("Directory(%s:%s): %s\n", parentFolder->parent->name, parentFolder->parent->next->name, parentFolder->path);
}
}*/
//else{
if(parentFolder->previous != NULL){
//printf("Directory(%x:%s): %s\n", parentFolder, parentFolder->previous->name, parentFolder->path);
}
//}

char *fullFileName;
struct stat statbuf; //Buffer for using stat()

while( (dirContent=readdir(currentDir)) ){ //Read the contents(just files and links) of the current directory
//We will ignore the folders "." and ".."
	if(strcmp(dirContent->d_name, ".") && strcmp(dirContent->d_name, "..")){
		fullFileName = calloc( strlen(dirContent->d_name)+strlen(parentFolder->path)+2, sizeof(char) );
		//perror("calloc");
		sprintf(fullFileName, "%s/%s", parentFolder->path, dirContent->d_name);

		if(lstat(fullFileName, &statbuf) != 0){
			//perror("lstat");
			printf("Stat failed for [%s]!\n", fullFileName);

		} else if(S_ISREG(statbuf.st_mode)){

			currentFile=populateFileInfo(&statbuf, currentFile, dirContent->d_name);
			currentFile->path = calloc( strlen(fullFileName)+1, sizeof(char) );
			strcpy(currentFile->path, fullFileName);
			//printf(" %s \n", dirContent->d_name );
			currentFile->parent=parentFolder;
			currentFile->isLink=FALSE;	//This is not a link file
			parentFolder->fileCount++;
			parentFolder->size+=currentFile->size;

		} else if(S_ISDIR(statbuf.st_mode)){

			//printf("\n");
			DIR *childDir;
			if( !(childDir=opendir(fullFileName)) ){
				//perror("opendir");
				printf("COULD NOT READ Directory: %s \n", fullFileName);
			}
			else{
				struct dirent *childContent;
				if(currentFolder->next != NULL){
					currentFolder=findFolderEnd(currentFolder);
				}
				currentFolder->next=populateFolderInfo(parentFolder, fullFileName, dirContent->d_name);
				currentFolder->next->previous=currentFolder;
				currentFolder=currentFolder->next;
				//parentFolder=currentFolder->parent;
				readDirContent(childContent, childDir, currentFolder, unsortedFiles, currentFile);
			}
			//parentFolder->fileCount++;
			//printf("\n\n");

		} else if(S_ISLNK(statbuf.st_mode)){
			//printf("Link:");
			currentFile=populateFileInfo(&statbuf, currentFile, dirContent->d_name);
			currentFile->path = calloc( strlen(fullFileName)+1, sizeof(char) );
			strcpy(currentFile->path, fullFileName);
			//printf(" %s \n", dirContent->d_name );
			currentFile->parent=currentFolder;
			currentFile->isLink=TRUE;	//This is a link file
			parentFolder->fileCount++;
			parentFolder->size+=currentFile->size;
		}
		else{
			//printf("Data:");
			currentFile=populateFileInfo(&statbuf, currentFile, dirContent->d_name);
			currentFile->path = calloc( strlen(fullFileName)+1, sizeof(char) );
			strcpy(currentFile->path, fullFileName);
			//printf(" %s \n", dirContent->d_name );
			currentFile->parent=currentFolder;
			currentFile->isLink=FALSE;	//This is an unknown file type
			parentFolder->fileCount++;
			parentFolder->size+=currentFile->size;
		}

		//printf(" %s \n", dirContent->d_name );
		free(fullFileName);
		//perror("free");
	}
}
//printf("Currents %d %i\n", (currentFolder->size/(1024)), currentFolder->fileCount);
closedir(currentDir);
}


void *findFolderEnd(struct FolderInfo *parent){
	while(parent->next != NULL){
		parent=parent->next;
	}
return parent;
}


void *findFileStart(struct FileInfo *file){
  if(file==NULL){
    printf("findFileStart: parameter is NULL\n");
    return NULL;
  }
	while(file->previous != NULL){
	  if(file==file->previous){
	    break;
	  }
		file=file->previous;
	}
return file;
}


void *findFileEnd(struct FileInfo *file){
  while(file->next != NULL){
    file=file->next;
  }
  return file;
}

void removeFileFromList(struct FileInfo *target){
  if(target->next==NULL){
    if(target->previous==NULL || target->previous->previous==target->previous){
      //Nothing to be done here
    }
    else{
      if(target->previous->previous == NULL || target->previous->previous == target->previous){
	target->previous->next=target->previous;
      }
      else{
	target->previous->next=NULL;
      }
    }
  }
  else{
    if(target->previous==NULL){
      target->next->previous=NULL;
    }
    else{
      target->next->previous=target->previous;
      target->previous->next=target->next;
    }
    target->next=NULL;
    target->previous=NULL;
  }
}

void addToList(struct FileInfo *temp, struct FileInfo *list){
  list->next=temp;
  temp->previous=list;
}

void *orderFiles(struct FileInfo *file){
struct FileInfo *sortedFiles, *temp;
file=findFileStart(file);
//sortedFiles=findFileStart(file);
 temp=file;

while(file->next != file && file!=NULL){
	if(temp->size < file->size){
		temp=file;
	}
	else{
		//file->previous->next=file->next;
		//file->next->previous=file->previous;

		if(file->next != NULL){
			file=file->next;
		}
		else{
		  if(sortedFiles==NULL){
		    sortedFiles=temp;
		  }
		  else{
		    sortedFiles=findFileEnd(sortedFiles);
		  }
		  //Remove from list
		  file=findFileStart(temp);
		  removeFileFromList(temp);
		  addToList(temp, sortedFiles);
		  //file=findFileStart(temp);
		}
	}
}

 return findFileStart(sortedFiles);
}

void *populateFolderInfo(struct FolderInfo *parent, char *path, char *name){
	struct FolderInfo *child;
	child=allocFolderStruct();
	child->path = calloc( strlen(path)+1, sizeof(char) );
	strcpy(child->path, path);
	child->name = calloc( strlen(name)+1, sizeof(char) );
	strcpy(child->name, name);
	child->parent=parent;
	child->previous=NULL;
	child->next=NULL;
	child->size=0;
	child->fileCount=0;
	return child;
}

void *populateFileInfo(struct stat *statStruct, struct FileInfo *currentFile, char *name){
	if(currentFile->next == currentFile){
		
	}
	else{
		currentFile->next=allocFileStruct(); //Allocate memory for the new struct
		currentFile->next->previous=currentFile;  //Set the new struct's previous to the old struct
		currentFile=currentFile->next;  //Before setting the currentFile to the new struct
	}
	currentFile->next=NULL;
	//printf("%x", currentFile);
	currentFile->size=(long double)statStruct->st_size;
	currentFile->name=calloc(strlen(name)+1, sizeof(char));
	//perror("calloc");
	strcpy(currentFile->name, name);
	return currentFile;
}



void *allocFileStruct(){
	return (struct FileInfo *) malloc(sizeof(struct FileInfo));
}

void *allocFolderStruct(){
	return (struct FolderInfo *) malloc(sizeof(struct FolderInfo));
}

void freeFolderStruct(struct FolderInfo *rootFolder){
long double totalSize=0;
long double fileCount=0;
long double folders=1;
	//printf("Self: %x Next: %x Previous: %x Name: %s Parent:%x\n", rootFolder, rootFolder->next, rootFolder->previous, rootFolder->name, rootFolder->parent);
printf("Searching for root folder...");
while(rootFolder->previous != NULL){
	rootFolder=rootFolder->previous;
}
printf("Complete\nFreeing structs...");

while(rootFolder->next != NULL){
//printf("%x\n", rootFolder->fileCount);
	//printf("Self: %x Next: %x Previous: %x Name: %s Parent:%x Size: %LF\n", rootFolder, rootFolder->next, rootFolder->previous, rootFolder->name, rootFolder->parent, rootFolder->size/1024);
	totalSize+=rootFolder->size;//+totalSize;
	fileCount+=rootFolder->fileCount;
	free(rootFolder->name);
	free(rootFolder->path);
	//if(rootFolder->next == rootFolder){
	//	rootFolder->next=NULL;}
	rootFolder=rootFolder->next;
	//free(rootFolder->previous);
	rootFolder->previous=NULL;
	folders++;
}

	//printf("\nSelf: %x Next: %x Previous: %x Name: %s Parent:%x Size: %LF\n", rootFolder, rootFolder->next, rootFolder->previous, rootFolder->name, rootFolder->parent, rootFolder->size/1024);
	totalSize+=rootFolder->size;//+totalSize;
	fileCount+=rootFolder->fileCount;
	free(rootFolder->name);
	free(rootFolder->path);
	free(rootFolder);
printf("Complete, Read %LF files, %LF folders totalling %LFMb\n", fileCount, folders, totalSize/(1024*1024));
}


void freeFileStruct(struct FileInfo *currentFile){
	//printf("Self: %x Next: %x Previous: %x Name: %s \n", currentFile, currentFile->next, currentFile->previous, currentFile->name);
printf("Searching for root file...");
currentFile=findFileStart(currentFile);

//while(currentFile->previous != NULL){
	//printf("Self: %x Next: %x Previous: %x Name: %s \n", currentFile, currentFile->next, currentFile->previous, currentFile->name);
//	currentFile=currentFile->previous;
//}


printf("Complete\nFreeing structs...");
//sleep(5);

while(currentFile->next != NULL){
	//printf("Self: %x Next: %x Previous: %x Name: %s Path: %s\n", currentFile, currentFile->next, currentFile->previous, currentFile->name, currentFile->path);
	//if(currentFile->name == currentFile){
	//	currentFile->next=NULL;
	//}
	free(currentFile->name);
	//perror("free");
	//if(strlen(currentFile->path) > 0){
	//printf("%s\n", currentFile->path);
	free(currentFile->path);
	//perror("free");
	//printf("%s\n", currentFile->path);
	//}
	//perror("free");
	currentFile=currentFile->next;
	free(currentFile->previous);
	//perror("free");
}

	//printf("Self: %x Next: %x Previous: %x Name: %s \n", currentFile, currentFile->next, currentFile->previous, currentFile->name);
	free(currentFile->name);
	free(currentFile->path);
	free(currentFile);

printf("Complete\n");
//sleep(6);
}
