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
	DIR *dir;
	long double fileCount;
	struct stat *info;
	struct FolderInfo *parent;
	struct FolderInfo *previous;
	struct FolderInfo *next;
	struct FileInfo *childFiles;
	struct FolderInfo *childFolders;
};

struct FileInfo {
	long double size;
	char *name;
	char *path;
	unsigned char isLink;
	struct stat *info;
	struct FolderInfo *parent;
	struct FileInfo *previous;
	struct FileInfo *next;
};


/**********************
Function Prototypes
*********************/

void *allocFileStruct();
void *allocFolderStruct();
void *populateFileInfo(struct stat *, struct FileInfo *);
void *populateFolderInfo(struct FolderInfo *, char *, char *);
void *findFolderEnd(struct FolderInfo *);
void *findFileStart(struct FileInfo *);
void *findFileEnd(struct FileInfo *);
void readDirContent(struct FolderInfo *);
void freeFileStruct(struct FileInfo *);
void freeFolderStruct(struct FolderInfo *);
void *orderFiles(struct FileInfo *);
void addToList(struct FileInfo *, struct FileInfo *);
void removeFileFromList(struct FileInfo *);
void checkFree(void *ptr);

int main( int argc, char **argv ){

	//DIR *rootDir, *currentDir;
	struct FolderInfo *rootFolder=NULL, *currentFolder;
	printf("Root struct %x:&%x\n", rootFolder, &rootFolder);
	rootFolder = allocFolderStruct();
	printf("Root struct %x:&%x\n", rootFolder, &rootFolder);

	if(argc != 2){ //Check for required number of arguments
	   printf("Usage: %s [path]\n", argv[0]);
		return 0;
	}
	
	if( !(rootFolder->dir=opendir(argv[1])) ){
		printf("Directory '%s' could not be opened!\n", argv[1]);
		return 0; //Exit due to error
	}
	closedir(rootFolder->dir);
	
	//currentDirent=readdir(rootDir);
	rootFolder->path = calloc( strlen(argv[1])+1, sizeof(char) );  //Allocate a string for the path to the root folder
	strcpy(rootFolder->path, argv[1]);  //Store the path
	printf("Path: %s\n", rootFolder->path);
	//rootFolder->name = calloc( strlen(rootFolder->dir->d_name)+1, sizeof(char) ); //Allocate space for the name
	//strcpy(rootFolder->name, rootFolder->dir->d_name);
	
	readDirContent(rootFolder);
	
	freeFolderStruct(rootFolder);  //Don't forget to clean up before we leave
	
	printf("End(%x:%x)\n", rootFolder->childFiles, rootFolder->childFiles->next);
	return 0;
}




void readDirContent(struct FolderInfo *folder){

//struct FolderInfo *currentFolder;
//currentFolder=parentFolder;

//struct stat statbuf; //Buffer for using stat()0

	//printf("Entering readDirContent\n");

	struct dirent *dirContent;
	folder->dir = opendir(folder->path); //Get us ready for some serious work
	struct FileInfo *currentFile;
	struct stat *statbuf;  //Buffer for using lstat
	char *fullFileName;

	while( (dirContent=readdir(folder->dir)) ){ //Read the contents(just files and links) of the current directory
		//We will ignore the folders "." and ".."
		if(strcmp(dirContent->d_name, ".") && strcmp(dirContent->d_name, "..")){
			fullFileName = calloc( strlen(dirContent->d_name)+strlen(folder->path)+2, sizeof(char) );
			sprintf(fullFileName, "%s/%s", folder->path, dirContent->d_name);
			statbuf = (struct stat *) malloc(sizeof(struct stat*));
			//printf("Located: %s ", fullFileName);
			if(lstat(fullFileName, statbuf) != 0){
				//perror("lstat");
				printf("Stat failed for [%s]!\n", fullFileName);
	
			} else if(S_ISREG(statbuf->st_mode)){
				//printf("Statted: %s\n", fullFileName);			
				if(folder->childFiles == NULL){ //We need to get this party started
					printf("Starting linked list\n");
					folder->childFiles = allocFileStruct();
					currentFile = folder->childFiles;
					currentFile->parent = folder;
				}
				else{ //Lets add a new file node
					printf("List: %x\nParent: %x\n", folder->childFiles, findFileEnd(folder->childFiles));
					currentFile = findFileEnd(folder->childFiles);
					currentFile->next = allocFileStruct();
					addToList( currentFile->next , currentFile);
					currentFile = currentFile->next;
					//currentFile->next->previous = currentFile;
				}
				
				currentFile->parent = folder;
				currentFile->name = calloc( strlen(dirContent->d_name)+1, sizeof(char) ); //Allocate a string for the name
				//sprintf(currentFile->name);
				strcpy( currentFile->name, dirContent->d_name );
				populateFileInfo( statbuf, currentFile );
				printf("\tFile: %s\n\tHex: %x\n", currentFile->name, currentFile);
			}			
			else if(S_ISDIR(statbuf->st_mode)){

				//printf("\n");
				if(folder->childFolders == NULL){
					printf("Starting childFolders linked list\n");
					folder->childFolders = allocFolderStruct();
					
				}
				
				DIR *childDir;
				if( !(childDir=opendir(fullFileName)) ){
					//perror("opendir");
					printf("COULD NOT READ Directory: %s \n", fullFileName);
				}
				else{
					//struct dirent *childContent;
					//if(currentFolder->next != NULL){
						//currentFolder=findFolderEnd(currentFolder);
					//}
					//currentFolder->next=populateFolderInfo(parentFolder, fullFileName, dirContent->d_name);
					//currentFolder->next->previous=currentFolder;
					//currentFolder=currentFolder->next;
					////parentFolder=currentFolder->parent;
					//readDirContent(childContent, childDir, currentFolder, unsortedFiles, currentFile);
				}
				//parentFolder->fileCount++;
				//printf("\n\n");
	
			} /* else if(S_ISLNK(statbuf->st_mode)){
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
			}*/
			
			 else{
				printf("Unknown type!\n");
			}
	
			//printf(" %s \n", dirContent->d_name );
			free(fullFileName);
			//free(statbuf);
			//perror("free");
		}
	}
	//printf("Currents %d %i\n", (currentFolder->size/(1024)), currentFolder->fileCount);
	closedir(folder->dir);
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
	if(file == NULL){
		printf("Error: NULL FileInfo pointer!\n");
		return NULL;
	}
	
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
	//list->next=temp;
	temp->previous=list;
	temp->parent = list->parent;
}


void addFolderToList(struct FolderInfo *temp, struct FolderInfo *list){
	//list->next=temp;
	temp->previous=list;
	temp->parent = list->parent;
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

void *populateFileInfo(struct stat *statStruct, struct FileInfo *currentFile){
	if(currentFile->next == currentFile){
		
	}
	else{
		//currentFile->next=allocFileStruct(); //Allocate memory for the new struct
		//currentFile->next->previous=currentFile;  //Set the new struct's previous to the old struct
		//currentFile=currentFile->next;  //Before setting the currentFile to the new struct
	}
	currentFile->next=NULL;
	//printf("%x", currentFile);
	currentFile->size=(long double)statStruct->st_size;
	//currentFile->name=calloc(strlen(name)+1, sizeof(char));
	//perror("calloc");
	//strcpy(currentFile->name, name);
	currentFile->info = statStruct; 
	return currentFile;
}



void *allocFileStruct(){
	struct FileInfo *temp = (struct FileInfo *) malloc(sizeof(struct FileInfo));
	temp->parent = NULL;
	temp->next = NULL;
	temp->previous = NULL;
	return temp;
}

void *allocFolderStruct(){
	struct FolderInfo *temp = (struct FolderInfo *) malloc(sizeof(struct FolderInfo));
	temp->parent = NULL;
	temp->previous = NULL;
	temp->next = NULL;
	temp->childFiles = NULL;
	temp->childFolders = NULL;
	temp->info = NULL;//= (struct stat *) malloc(sizeof(struct stat));
	temp->dir = NULL;
	temp->path = NULL;
	temp->name = NULL;
	//temp->dir = (DIR *) malloc(sizeof(DIR *));
	
	//return (struct FolderInfo *) malloc(sizeof(struct FolderInfo));
	return temp;
}



void freeFolderStruct(struct FolderInfo *rootFolder){
	struct FolderInfo *currentFolder = rootFolder;
	struct FolderInfo *tmp = currentFolder;
	printf("Cleanup time, directory %x:&%x\n", rootFolder, &rootFolder);
	//printf("");
	//freeFileStruct(tmp->childFiles);
    /*
	while(currentFolder != NULL){
		tmp = currentFolder;
		while(tmp->childFolders != NULL){  //Look for the last child folder
			tmp = tmp->childFolders;  //Move the pointer along
		}
		while(tmp->next != NULL){  //Read through the list of siblings
			tmp = tmp->next;  //Move our pointer along
		}
		if(tmp->childFolders != NULL){  //Make sure there are not more files left
			freeFolderStruct(tmp);  //w00t for recursive code!
		}
		if(tmp->previous != NULL){
			currentFolder=tmp->previous;
		}
		else if(tmp->parent != NULL){
			currentFolder=tmp->parent;
		}
		else{
			//printf("Removing root folder[%s]\n", tmp->name);
			currentFolder=tmp; //We are at the end of the line
		}
		printf("1\n");
		//freeFileStruct(tmp->childFiles);
		printf("Path (%x:&%x)\n", tmp->path, &tmp->path);
		checkFree(tmp->path);
		printf("3\n");
		printf("Dir (%x:&%x)\n", tmp->dir, &tmp->dir);
		//checkFree(tmp->dir);
		printf("4\n");
		checkFree(tmp->info);
		printf("5\n");
		//checkFree(tmp->path);
		printf("6\n");
		checkFree(tmp);
		printf("7\n");
	}
	*/
}


void checkFree(void *ptr){
	printf("[%x:&%x]\n", ptr, &ptr);
	if(ptr != NULL){
		free(ptr);
	}
	else{
		printf("No free\n");
		//ptr cannot be freed
		//becuase ptr has not
		//been malloc'ed
	}
	ptr = NULL;
	printf("[%x:&%x]!\n", ptr, &ptr);
}

void freeFileStruct(struct FileInfo *currentFile){
	printf("Searching for root file...");
	checkFree(currentFile);
	if(currentFile == NULL){
		printf("No files to free\n");
		return;
	}
	currentFile=findFileStart(currentFile);
	while(currentFile != NULL){
		free(currentFile->name);
		free(currentFile->path);
		free(	currentFile->info);
		if(currentFile->next == NULL){
			free(currentFile);
			currentFile = NULL;
		}
		else{
			currentFile=currentFile->next;
			free(currentFile->previous);
			currentFile->previous = NULL;
		}
	}
}
