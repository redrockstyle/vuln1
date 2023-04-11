// Файл files.h.
// Заголовочный файл модуля files.c.
//
// Зачётное задание 1.
//
// Маткин Илья Александрович    04.05.2015

#ifndef _UTILS_FILES_H_
#define _UTILS_FILES_H_

#include <stddef.h>
#include <dirent.h>

//----------------------------------------

#define MAX_STRING_SIZE 241
#define ID_SIZE 40

#define META_DIR    "/meta/"
#define FILES_DIR   "/files/"
#define BACKUP_DIR  "/backup_files/"

#define IN
#define OUT

typedef int bool;
#define TRUE    1
#define FALSE   0

//----------------------------------------

size_t TGetString (int fd, OUT char *dst);
void TPutString (int fd, IN const char *src);
void TFlushString (int fd);

char * TParseParam (IN OUT char *str);
char * TGetMetaParam (IN const char *id, IN const char *paramName, OUT char *buf);

bool TGetNextId (IN DIR *dir, OUT char *outId);

void TGenerateId (OUT char *dstId);

void TGetMetaPath (IN const char *id, OUT char *dstPath);
void TGetFilesPath (IN const char *fileName, OUT char *dstPath);
void TGetBackupPath (IN const char *fileName, OUT char *dstPath);

size_t TGetFileSize (IN const char *id);
void TGetFileNameById (IN const char *id, OUT char *dstFileName);
bool TGetIdByFileName (IN const char *fileName, OUT char *dstId);

void TGenerateRandomString (unsigned int count, OUT char *dst);
int TCheckString (IN const char *str);

//----------------------------------------



#endif  // _UTILS_FILES_H_

