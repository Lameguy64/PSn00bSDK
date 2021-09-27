#ifndef _FILELIST_H
#define _FILELIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

typedef struct {
	char*	fileName;
    char*	aliasName;
    int		windowSize;
    int		hash1Size;
    int		hash2Size;
} FileListEntry;

class FileListClass {

	int				NumFiles;
	int				AllocFiles;
	FileListEntry*	FileList;

public:

    FileListClass();
    virtual ~FileListClass();

    void AddFileEntry(const char* fileName, const char* aliasName, short windowSize, short hash1Size, short hash2Size);

    const FileListEntry* Entry(int index);
    int EntryCount();

    void PrintEntries();

};


#endif
