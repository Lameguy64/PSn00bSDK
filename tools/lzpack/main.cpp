#include <stdio.h>
#include <tinyxml2.h>

#include "lzp/lzconfig.h"
#include "lzp/lzp.h"
#include "filelist.h"


#define BUFF_SIZE	4096


typedef struct {
	char			id[3];
	unsigned char	numFiles;
} QLP_HEAD;

typedef struct {
	char			fileName[16];
	unsigned int	fileSize;
	unsigned int	offset;
} QLP_FILE;


typedef struct {
	char			name[16];
	unsigned int	size;
	unsigned int	offset;			// In 2048 byte sector units
} PCK_FILE;

typedef struct {
	char		id[3];
	u_char		numFiles;
	PCK_FILE	file[85];	// File entries
	int			lba;		// LBA of the PCK file (in 2048 byte sector units)
} PCK_TOC;


namespace param {

	bool	AlwaysOverwrite		= false;
	char	ScriptFile[MAX_PATH]= { 0 };

}


int ParseCreateElement(tinyxml2::XMLElement* element);

char* lcase(char* str);
const char* TrimPathName(const char* path);


int main(int argc, const char* argv[]) {

    printf("LZPack v0.61b - File Compression and Packing Utility\n");
    printf("2016-2019 Meido-Tek Productions (Lameguy64)\n\n");

	if (argc <= 1) {

		printf("Parameters:\n");
		printf("   lzpack [-y] <scriptFile>\n\n");
		printf("   -y           - Always overwrite existing files.\n");
		printf("   <scriptFile> - Script file to parse (in XML format, see readme.txt).\n");

		exit(0);

	}


	// Parse arguments
	for(int i=1; i<argc; i++) {

        if (strcmp("-y", argv[i]) == 0) {

			param::AlwaysOverwrite = true;

        } else if ((argv[i][0] == '-') || (argv[i][0] == '/')) {

			printf("Unknown parameter: %s\n", argv[i]);

        } else {

        	strcpy(param::ScriptFile, argv[i]);

        }

	}

	if (strlen(param::ScriptFile) == 0) {
		printf("ERROR: No script file specified.\n");
		exit(EXIT_FAILURE);
	}


	tinyxml2::XMLDocument document;

	switch(document.LoadFile(param::ScriptFile)) {
	case tinyxml2::XML_SUCCESS:

		break;

	case tinyxml2::XML_ERROR_FILE_NOT_FOUND:

		printf("ERROR: Could not find file: %s\n", param::ScriptFile);
		exit(EXIT_FAILURE);

	case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
	case tinyxml2::XML_ERROR_FILE_READ_ERROR:

		printf("ERROR: Could not load file: %s\n", param::ScriptFile);
		exit(EXIT_FAILURE);

	case tinyxml2::XML_ERROR_EMPTY_DOCUMENT:

		printf("ERROR: %s is empty.\n", param::ScriptFile);
		exit(EXIT_FAILURE);

	default:

		printf("ERROR: Unknown error when loading %s\n", param::ScriptFile);
		exit(EXIT_FAILURE);

	}


    tinyxml2::XMLElement* element = document.FirstChildElement("lzp_project");

	if (element == NULL) {

		printf("ERROR: <lzp_project> element not found.\n");
		exit(EXIT_FAILURE);

	}


	tinyxml2::XMLElement* createElement = element->FirstChildElement("create");

	while(createElement != NULL) {

		ParseCreateElement(createElement);

		createElement = createElement->NextSiblingElement();

	}


    return(0);

}


int CreateLZPfile(const char* packFile, FileListClass* fileList) {

	FILE*		packp;
	LZP_FILE	entry[fileList->EntryCount()];
	int			overallSize=0;
	int			overallPackedSize=0;


    packp = fopen(packFile, "wb");

    fseek(packp, sizeof(LZP_HEAD)+(sizeof(LZP_FILE)*fileList->EntryCount()), SEEK_SET);

	for(int i=0; i<fileList->EntryCount(); i++) {

        const char* name;

        if (fileList->Entry(i)->aliasName == NULL) {

			name = TrimPathName(fileList->Entry(i)->fileName);

        } else {

        	name = fileList->Entry(i)->aliasName;

        }

        if (strlen(name) > 15) {

            printf("ERROR: Entry '%s' has more than 15 characters.\n", name);
            fclose(packp);
            unlink(packFile);

            return(0);

        }

		strcpy(entry[i].fileName, name);

		if (fileList->Entry(i)->aliasName == NULL) {
			printf("   Packing %s... ", fileList->Entry(i)->fileName);
		} else {
			printf("   Packing %s as %s... ", fileList->Entry(i)->fileName, fileList->Entry(i)->aliasName);
		}


		FILE*	fp = fopen(fileList->Entry(i)->fileName, "rb");

		fseek(fp, 0, SEEK_END);
        int fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

        void* fileBuff = malloc(fileSize);
		fread(fileBuff, fileSize, 1, fp);

		fclose(fp);


        void*	compBuff = malloc(fileSize+16384);
        int		compSize = lzCompress(compBuff, fileBuff, fileSize, 2);


        entry[i].crc		= lzCRC32(compBuff, compSize, LZP_CRC32_REMAINDER);
		entry[i].fileSize	= fileSize;
		entry[i].packedSize	= compSize;
        entry[i].offset		= ftell(packp);

        fwrite(compBuff, compSize, 1, packp);

        free(compBuff);
        free(fileBuff);

		printf("Ok. (%.02f%%)\n", 100.f*((float)compSize/fileSize));

		overallSize			+= fileSize;
		overallPackedSize	+= compSize;

	}


    LZP_HEAD head;

    strncpy(head.id, "LZP", sizeof(head.id));
    head.numFiles = fileList->EntryCount();

	fseek(packp, 0, SEEK_SET);
	fwrite(&head, sizeof(LZP_HEAD), 1, packp);

    fwrite(entry, sizeof(LZP_FILE), fileList->EntryCount(), packp);

	fclose(packp);


    printf("Packed %d file(s) totaling %d bytes (%.02f%% compression ratio).\n",
		fileList->EntryCount(),
		overallPackedSize,
		100.f*((float)overallPackedSize/overallSize)
	);


	return(true);

}

int CreateQLPfile(const char* packFile, FileListClass* fileList) {

    FILE*		packp;
	QLP_HEAD	head;
	QLP_FILE	fileEntry[fileList->EntryCount()];

	strncpy(head.id, "QLP", 3);
	head.numFiles = fileList->EntryCount();

	packp = fopen(packFile, "wb");

	fseek(packp, sizeof(QLP_HEAD)+(sizeof(QLP_FILE)*head.numFiles), SEEK_SET);

	for(int i=0; i<head.numFiles; i++) {

		// Get name for the file entry either from its source file name or specified alias
		const char* name;

        if (fileList->Entry(i)->aliasName == NULL) {

			name = TrimPathName(fileList->Entry(i)->fileName);

        } else {

        	name = fileList->Entry(i)->aliasName;

        }

		// Make sure entry name does not exceed 15 characters (16 with null terminator byte)
        if (strlen(name) > 15) {

            printf("ERROR: Entry '%s' has more than 15 characters.\n", name);
            fclose(packp);
            unlink(packFile);

            return(0);

        }

		strcpy(fileEntry[i].fileName, name);

		if (fileList->Entry(i)->aliasName == NULL) {

			printf("   Packing %s... ", fileList->Entry(i)->fileName);

		} else {

			printf("   Packing %s as %s... ", fileList->Entry(i)->fileName, fileList->Entry(i)->aliasName);

		}

		// Make sure written data is aligned in multiples of 4 bytes
		if ((4*((ftell(packp)+3)/4)) != ftell(packp))
			fseek(packp, (4*((ftell(packp)+3)/4)), SEEK_SET);

		// Set name and offset of file entry
		memset(fileEntry[i].fileName, 0x00, 16);
		strcpy(fileEntry[i].fileName, name);
		fileEntry[i].offset = ftell(packp);

		// Open file and copy its contents to the pack file
		FILE* fp = fopen(fileList->Entry(i)->fileName, "rb");

		int bytesCopied = 0;
		void* copyBuff = malloc(BUFF_SIZE);

		while(!feof(fp)) {

			int bytesRead = fread(copyBuff, 1, BUFF_SIZE, fp);

			fwrite(copyBuff, bytesRead, 1, packp);

            bytesCopied += bytesRead;

		}

		free(copyBuff);
		fclose(fp);

		fileEntry[i].fileSize = bytesCopied;

		printf("Done.\n");

	}

	printf("Packed %d file(s) totaling %d bytes.\n", head.numFiles, (int)ftell(packp));

	fseek(packp, 0, SEEK_SET);
	fwrite(&head, sizeof(QLP_HEAD), 1, packp);

	fwrite(fileEntry, sizeof(QLP_FILE), head.numFiles, packp);

	fclose(packp);

	return(true);

}

int CreatePCKfile(const char* packFile, FileListClass* fileList) {

	FILE*	packp;
	PCK_TOC	toc;

	memset(&toc, 0x00, sizeof(PCK_TOC));

	toc.numFiles = fileList->EntryCount();

    packp = fopen(packFile, "wb");

	fseek(packp, 2048, SEEK_SET);

	for(int i=0; i<toc.numFiles; i++) {

		// Get name for the file entry either from its source file name or specified alias
		const char* name;

        if (fileList->Entry(i)->aliasName == NULL) {

			name = TrimPathName(fileList->Entry(i)->fileName);

        } else {

        	name = fileList->Entry(i)->aliasName;

        }

		// Make sure entry name does not exceed 15 characters (16 with null terminator byte)
        if (strlen(name) > 15) {

            printf("ERROR: Entry '%s' has more than 15 characters.\n", name);
            fclose(packp);
            unlink(packFile);

            return(0);

        }

		strcpy(toc.file[i].name, name);
		toc.file[i].offset = ftell(packp)/2048;

		if (fileList->Entry(i)->aliasName == NULL) {

			printf("   Packing %s... ", fileList->Entry(i)->fileName);

		} else {

			printf("   Packing %s as %s... ", fileList->Entry(i)->fileName, fileList->Entry(i)->aliasName);

		}

		FILE* fp = fopen(fileList->Entry(i)->fileName, "rb");
		void* buff = malloc(BUFF_SIZE);

		int bytesTotal = 0;

        while(!feof(fp)) {

			int bytesRead = fread(buff, 1, BUFF_SIZE, fp);
            fwrite(buff, 1, bytesRead, packp);
			bytesTotal += bytesRead;

        }

		fclose(fp);
		free(buff);

		toc.file[i].size = bytesTotal;

		if ((2048*((ftell(packp)+2047)/2048)) != ftell(packp)) {

			int pad = (2048*(((ftell(packp)%2048)+2047)/2048))-(ftell(packp)%2048);
			char padding[pad];

			memset(padding, 0x00, pad);
			fwrite(padding, pad, 1, packp);

		}

		printf("Done.\n");

	}

    printf("Packed %d file(s) totaling %d bytes.\n", toc.numFiles, (int)ftell(packp));

	strncpy(toc.id, "PCK", 3);

    fseek(packp, 0, SEEK_SET);
    fwrite(&toc, sizeof(PCK_TOC), 1, packp);

    fclose(packp);

	return(true);

}

int ParseCreateElement(tinyxml2::XMLElement* element) {


	const char* packName = element->Attribute("packname");

    if (packName == NULL) {
		printf("ERROR: No 'packname' attribute found in <create> element.\n");
		return(false);
    }


	int	packFormat;

	{

		char*		packType = (char*)element->Attribute("format");

		if (packType == NULL) {

			packType = strdup("lzp");

		} else {

			packType = strdup(packType);
			lcase(packType);

		}

		if (strcmp("lzp", packType) == 0) {
			packFormat = 0;
		} else if (strcmp("qlp", packType) == 0) {
			packFormat = 1;
		} else if (strcmp("pck", packType) == 0) {
			packFormat = 2;
		} else {

			printf("ERROR: Unknown pack format: %s\n", packType);
			free(packType);
			return(false);

		}

		free(packType);

	}


	printf("Creating %s in ", packName);
	switch(packFormat) {
	case 0:

		printf("LZP");
		break;

	case 1:

		printf("QLP");
		break;

	case 2:

		printf("PCK");
		break;

	}
	printf(" format...\n");


	tinyxml2::XMLElement* fileElement = element->FirstChildElement("file");

	if (fileElement == NULL) {
		printf("ERROR: No file element(s) found.\n");
		return(false);
	}


    FileListClass fileList;

    while(1) {

		bool	valid = true;

		int		entryWindowSize = LZP_WINDOW_SIZE;
		int		entryHash1Size	= LZP_HASH1_SIZE;
		int		entryHash2Size	= LZP_HASH2_SIZE;

		if (fileElement->GetText() == NULL) {
			printf("WARNING: <file> element not containing text found.\n");
			valid = false;
		}


		FILE* fp = fopen(fileElement->GetText(), "rb");
		if (!fp) {
			printf("WARNING: File '%s' either does not exist or it cannot be opened.\n", fileElement->GetText());
			valid = false;
		}
		fclose(fp);


		if (valid) {
			fileList.AddFileEntry(
				fileElement->GetText(),
				fileElement->Attribute("alias"),
				entryWindowSize,
				entryHash1Size,
				entryHash2Size
			);
		}


		fileElement = fileElement->NextSiblingElement();

		if (fileElement == NULL)
			break;

    }


	if (fileList.EntryCount() == 0) {
		printf("No file(s) to pack.\n");
		return(true);
	}

	switch(packFormat) {
	case 0:	// Create LZP
		CreateLZPfile(packName, &fileList);
		break;
	case 1:	// Create QLP
		CreateQLPfile(packName, &fileList);
		break;
	case 2:	// Create PCK
		CreatePCKfile(packName, &fileList);
		break;
	}


    return(true);

}


const char* TrimPathName(const char* path) {

    if ((strrchr(path, '\\') == NULL) && (strrchr(path, '/') == NULL)) {

        return(path);

    } else {

		if (strrchr(path, '\\') == NULL)
			return(strrchr(path, '/')+1);

		return(strrchr(path, '\\')+1);

    }

}

char* lcase(char* str) {

	for(int i=0; str[i]!=0x00; i++)
		str[i] = tolower(str[i]);

	return(str);

}
