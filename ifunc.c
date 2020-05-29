#include "ifunc.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

const char *errorDesc[] = {
    [EMPTYINFILE] = "Infile is empty!",
    [INVALIDAGRSNUM] = "Program provided with invalid number of arguments!",
    [EEXIT] = "Early exit... Bye :)",
    [MEMERROR] = "Memory allocation error!",
    [FILENEXISTS] = "File not exists!"
};


char inputFile[MAX_FILENAME];
char outputFile[MAX_FILENAME];
int iFD;
FILE *iFP;

off_t inspect_inputFile() {
    iFD = open(inputFile, O_RDONLY);
    if(iFD < 0) {
        close(iFD);
        return 0;
    }
    off_t sz = lseek(iFD, 0L, SEEK_END);
    if(sz == 0) {
        close(iFD);
        return sz;
    } else {
        printf("Size of file to proceed: %ld\n", sz);
        lseek(iFD, 0L, SEEK_SET);
        close(iFD);

        iFP = fopen(inputFile, "r");
    }

    return sz;
}

bool inspect_filename(char filename[MAX_FILENAME]) {
    char *symbol = filename;
    while(*symbol != '\0') {
        if(isalpha(*symbol)) return true;
        symbol++;
    }
    return false;
}

int get_input(int argc, char **argv){
    if(argc > 2) {
        puts(errorDesc[INVALIDAGRSNUM]);
        return INVALIDAGRSNUM;
    } else if(argc == 2) {
        inputFile[0] = '\0';
        strcpy(inputFile, argv[1]);

        short int tres = get_input_from_file();
        if(tres != SUCCESS) {
            puts(errorDesc[tres]);
            return tres;
        }

        return SUCCESS;
    } else {
        puts("- Press 1 to enter the filename");
        puts("- Press 2 to enter the code from keyboard");
        puts("- Press any other digit to exit");
        printf("Choose the option:\t");

        int digit = 0;
        scanf("%d", &digit);
        getchar();
        if(digit == 1) {

            char filename[MAX_FILENAME] ;
            printf("Enter the filename: ");

            char *res = fgets(filename, MAX_FILENAME, stdin);
            filename[strlen(filename)-1] = '\0';
            iFP = fopen(filename, "r");
            if(iFP == NULL) {
                puts(errorDesc[FILENEXISTS]);
                return FILENEXISTS;
            }
            if(res == NULL || !inspect_filename(filename)) {
                puts("Invalid filename!");
                return(get_input(argc, argv));
            }   
            strcpy(inputFile, filename);

            short int tres = get_input_from_file();
            if(tres != SUCCESS) {
                puts(errorDesc[tres]);
                return tres;
            }

            return SUCCESS;

        } else if (digit == 2) {
            short int tres = get_input_from_keyboard();
            if(tres != SUCCESS){
                puts(errorDesc[tres]);
                return tres;
            }

            return SUCCESS;
        } else {
            puts(errorDesc[EEXIT]);
            return EEXIT;
        }
    }

    return SUCCESS;
}

int get_input_from_keyboard() {
    printf("Enter the code manually (^D to EOF): \n");
    source = (char *)calloc(1024, sizeof(char));
    if(source == NULL) {
        puts(errorDesc[MEMERROR]);
        return MEMERROR;
    }
    char line[MAX_INPUT_SIZE];
    memset(line, 0, MAX_INPUT_SIZE);
    int rc;
    while((rc = scanf("%c", line)) != EOF) {
	    if(rc == 1) {
            strcat(source, line);
        }
        line[0] = '\0';
    }
    source[strlen(source)] = '\0';

    return SUCCESS;
}

int get_input_from_file() {
    //check whether it's empty or not;
    size_t fileSize = (size_t) inspect_inputFile();
    if(fileSize == 0) {
        fclose(iFP);
        return EMPTYINFILE;
    }

    source = (char *)calloc(fileSize, sizeof(char));
    if(source == NULL) {
        fclose(iFP);
        return MEMERROR;
    }
    // read the file into the source
    memset(source, 0, fileSize);
    fread(source, sizeof(char), fileSize, iFP);
    fclose(iFP);

    return SUCCESS;
}
