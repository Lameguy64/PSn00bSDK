#include "filelist.h"

FileListClass::FileListClass() {

	NumFiles = 0;
	AllocFiles = 1;

	FileList = (FileListEntry*)malloc(sizeof(FileListEntry));
	memset(FileList, 0x00, sizeof(FileListEntry));

}

FileListClass::~FileListClass() {

	for(int i=NumFiles-1; i>=0; i--) {

		if (FileList[i].fileName != NULL)
			free(FileList[i].fileName);

		if (FileList[i].aliasName != NULL)
			free(FileList[i].aliasName);

	}

	free(FileList);

}

void FileListClass::AddFileEntry(const char* fileName, const char* aliasName, short windowSize, short hash1Size, short hash2Size) {

	if (NumFiles >= AllocFiles) {

		FileList = (FileListEntry*)realloc(FileList, sizeof(FileListEntry)*(AllocFiles+1));
		memset(&FileList[AllocFiles], 0x00, sizeof(FileListEntry));

		AllocFiles++;

	}

	if (aliasName == NULL)
		FileList[NumFiles].aliasName = NULL;
	else
		FileList[NumFiles].aliasName = strdup(aliasName);

	FileList[NumFiles].fileName		= strdup(fileName);
	FileList[NumFiles].windowSize	= windowSize;
	FileList[NumFiles].hash1Size	= hash1Size;
	FileList[NumFiles].hash2Size	= hash2Size;
	NumFiles++;

}

const FileListEntry* FileListClass::Entry(int index) {

	return(&FileList[index]);

}

int FileListClass::EntryCount() {

	return(NumFiles);

}

void FileListClass::PrintEntries() {

	for(int i=0; i<NumFiles; i++) {

		printf("FL  FILE:%s", FileList[i].fileName);

		if (FileList[i].aliasName != NULL) {
			printf("  ALIAS:%s\n", FileList[i].aliasName);
		} else {
			printf("\n");
		}

	}

}
