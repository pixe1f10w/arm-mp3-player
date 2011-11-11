#ifndef _FILE_MANAGEMENT_H_
#define _FILE_MANAGEMENT_H_
//the number of files
extern unsigned char ucFileCount;
//index of the file
extern unsigned char ucFileIndex;
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
