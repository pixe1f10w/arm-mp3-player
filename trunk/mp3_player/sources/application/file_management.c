#include <string.h>

#include "file_management.h"
//
//Save file system
static FATFS sFatFs;
static DIR sDirObject;
static FILINFO sFileInfo;
/**File Object for open/read file*/
FIL sSongFileObject;
SongInfoHeader sSongInfoHeader;
char *sFilePath;
unsigned long  MinsCurrent;
unsigned long  SecondCurrent;

char pcFiles[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];
//the number of files
unsigned char ucFileCount =0;
//index of the file
unsigned char ucFileIndex =0;
//Path current Dir
#define MAX_PATH_STRING_LEN 50
static char pcPath[MAX_PATH_STRING_LEN]="0:/";
static unsigned char ucEndDirPath=3;

//Store for browsing
Item sItemList[NUM_LIST_STRINGS];
unsigned char ucItemCount=0;
unsigned char ucItemIndex =0;
//File Path
static char pcFilePath[MAX_PATH_STRING_LEN+MAX_FILENAME_STRING_LEN];
static unsigned char ucEndFilePath=3;
/**
*
*/
char CheckExtension(char *fileName){
  char i=0;
  while(fileName[i]!='.'&&i<20)
    i++;
  if(i==20)
    return NOT_SUPPORT_FORMAT;

  if(strcmp(fileName+i,".wav")==0||
                  strcmp(fileName,".WAV")==0)
    return WAV_FILE;
  else if(strcmp(fileName+i,".mp3")==0||
                      strcmp(fileName+i,".MP3")==0)
    return MP3_FILE;
  else
    return NOT_SUPPORT_FORMAT;
    
}
//******************************************************************************
//
// This function is called to read the contents of the current directory on
// the SD card and fill the pcFiles containing the names of mp3/wav files.
//
//******************************************************************************
int ListFiles(void)
{
    FRESULT fresult;
    unsigned char i;
    //
    // Open the current directory for access.
    //
    fresult = f_opendir(&sDirObject, pcPath);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        //
        // Ensure that the error is reported.
        //
        return(fresult);
    }
    //
    strcpy(pcFilePath,pcPath);
    ucEndFilePath = ucEndDirPath;
    ucFileCount = 0;
    //
    // Enter loop to enumerate through all directory entries.
    //
    while(1)
    {
        //
        // Read an entry from the directory.
        //
        fresult = f_readdir(&sDirObject, &sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(fresult != FR_OK)
        {
            return(fresult);
        }

        //
        // If the file name is blank, then this is the end of the
        // listing.
        //
        if(!sFileInfo.fname[0])
        {
            break;
        }

        //
        // Add the information as a line in the listbox widget.
        //
        if(ucFileCount < NUM_LIST_STRINGS)
        {
            //
            // Ignore directories.
            //
            if((sFileInfo.fattrib & AM_DIR) == 0)
            {
                i=0;
                while(sFileInfo.fname[i]!='.' && sFileInfo.fname[i] != 0)
                  i++;
                if(strcmp(sFileInfo.fname+i,".wav")==0||
                   strcmp(sFileInfo.fname+i,".WAV")==0||
                     strcmp(sFileInfo.fname+i,".mp3")==0||
                      strcmp(sFileInfo.fname+i,".MP3")==0)
                {
                  strcpy(pcFiles[ucFileCount], sFileInfo.fname);
                  ucFileCount++;
                }
            }
        }
    }
    if(ucFileCount)
    {
      if(ucEndFilePath==3)
        ucEndFilePath--;
      pcFilePath[ucEndFilePath]='/';
      strcpy(pcFilePath+ucEndFilePath+1,pcFiles[0]);
    }
    ucItemIndex=0;
    //
    // Made it to here, return with no errors.
    //
    return(0);
}
char * getCurrentFile(void)
{
  return pcFilePath;
}
int ListDirItems(void)
{
  FRESULT fresult;
    //
    // Open the current directory for access.
    //
    fresult = f_opendir(&sDirObject, pcPath);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        //
        // Ensure that the error is reported.
        //
        return(fresult);
    }
    //
    ucItemCount=0;
    //
    // Enter loop to enumerate through all directory entries.
    //
    while(1)
    {
        //
        // Read an entry from the directory.
        //
        fresult = f_readdir(&sDirObject, &sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(fresult != FR_OK)
        {
            return(fresult);
        }

        //
        // If the file name is blank, then this is the end of the
        // listing.
        //
        if(!sFileInfo.fname[0])
        {
            break;
        }

        //
        // Add the information as a line in the listbox widget.
        //
        if(ucItemCount < NUM_LIST_STRINGS)
        {
            //
            // Ignore directories.
            //
            sItemList[ucItemCount].fattrib=sFileInfo.fattrib;
            strcpy(sItemList[ucItemCount].fname,sFileInfo.fname);
            ucItemCount++;
        }
    }
    ucItemIndex =0;
    //
    // Made it to here, return with no errors.
    //
    return(0);
}
/**
*
*/
int BackParentDir(void)
{
  if(ucEndDirPath==3)
    return 1;
  //Back slash
  while(ucEndDirPath>3 &&pcPath[ucEndDirPath] !='/')
    ucEndDirPath--;
  //End string
  pcPath[ucEndDirPath]=0;
  //List Items in Parent Dir
  ListDirItems();
  return 0;
}
/**
*
*/
void OpenChooseDir(void)
{
  //Ignore file
  if((sItemList[ucItemIndex].fattrib& AM_DIR) == 0)
    return;
  //
  if(ucEndDirPath > (MAX_PATH_STRING_LEN-MAX_FILENAME_STRING_LEN))
    return;
  if(ucEndDirPath==3)
    ucEndDirPath--;
    
  pcPath[ucEndDirPath]='/';
  strcpy(&pcPath[ucEndDirPath]+1,sItemList[ucItemIndex].fname);
  //
  ucEndDirPath +=strlen(sItemList[ucItemIndex].fname)+1;
  //List Items in Parent Dir
  ListDirItems();
}
void NextItem(void)
{
  ucItemIndex++;
  if(ucItemIndex >= ucItemCount)
    ucItemIndex = 0;
}
void PreItem(void)
{
  ucItemIndex--;
  if(ucItemIndex >= ucItemCount)
    ucItemIndex = ucItemCount -1;
}
/**
*
*/
char * preFile(void)
{
  if(ucFileCount)
  {
    ucFileIndex--;
    if(ucFileIndex >= ucFileCount)
      ucFileIndex = ucFileCount -1;
    //Back slash
    if(ucEndFilePath==3)
      ucEndFilePath--;
    while(ucEndFilePath>3&&pcFilePath[ucEndFilePath] !='/')
      ucEndFilePath--;
    strcpy(&pcFilePath[ucEndFilePath]+1,pcFiles[ucFileIndex]);
    return pcFilePath;
  }
  else
    return 0;
}
/**
*
*/
char * nextFile(void)
{
  if(ucFileCount)
  {
    ucFileIndex++;
    if(ucFileIndex >= ucFileCount)
      ucFileIndex = 0;
    //Back slash
    if(ucEndFilePath==3)
      ucEndFilePath--;
    while(ucEndFilePath>3&&pcFilePath[ucEndFilePath] !='/')
      ucEndFilePath--;
    strcpy(&pcFilePath[ucEndFilePath]+1,pcFiles[ucFileIndex]);
    return pcFilePath;
  }
  else
    return 0;
}
char InitFileManagement(void)
{
  FRESULT fresult;
  //
    // Mount the file system, using logical disk 0.
    //
    fresult = f_mount(0, &sFatFs);
    if(fresult != FR_OK)
    {
        return(1);
    }
    return 0;
}