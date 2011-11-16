#ifndef _FILE_MANAGEMENT_H_
#define _FILE_MANAGEMENT_H_

//Store Filenames for Play
#define MAX_FILENAME_STRING_LEN (8 + 1 + 3 + 1)
#define NUM_LIST_STRINGS 50
typedef struct ITEM{
  unsigned char fattrib;
  char fname[MAX_FILENAME_STRING_LEN];
}Item;
//**********Files List***********
extern char pcFiles[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];;
//the number of files
extern unsigned char ucFileCount;
//index of the file
extern unsigned char ucFileIndex;
//*****Store for browsing********
extern Item sItemList[NUM_LIST_STRINGS];
extern unsigned char ucItemCount;
extern unsigned char ucItemIndex;
//*******************************
char * preFile(void);
char * nextFile(void);
int ListDirItems(void);
int ListFiles(void);
char * getCurrentFile(void);
int BackParentDir(void);
void OpenChooseDir(void);
void NextItem(void);
void PreItem(void);
#endif //_FILE_MANAGEMENT_H_
