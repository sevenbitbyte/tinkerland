#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#define LIGHT_FILE  "/sys/devices/platform/thinkpad_acpi/leds/tpacpi::thinklight/brightness"
#define LIGHT_ON    "1"
#define LIGHT_OFF   "0"
#define MICRO_TO_MILLI(x)(x*1000.0f)

void writeToFile(char *data){
    FILE *filePtr;
    filePtr=fopen(LIGHT_FILE, "w");
    if(filePtr==NULL){
        perror("fopen");
        exit(-2);
    }
    fprintf(filePtr, "%s\n", data);
    fclose(filePtr);
}

void printUsage(char* progName){
    printf("Usage: %s [option]\n", progName);
    printf("\t-l loop\tNumber of times to blink.\n");
    printf("\t-t time\tTimeout between blinks in ms.\n");
    printf("\t-h\tPrints this usage.\n\n");
}

int main(int argc, char* argv[]){

    if(argc==1){
        printUsage(argv[0]);
        exit(-1);
    }

    char options[]="l:t:h";
    int flag;
    unsigned int blinks=3;
    float timeout=MICRO_TO_MILLI(20.0f);

    while((flag=getopt(argc, argv, options)) != -1){
        switch(flag){
            case 'h':
                printUsage(argv[0]);
                exit(-1);
            case 'l':
                blinks=atoi(optarg);
                break;
            case 't':
                timeout=MICRO_TO_MILLI(atof(optarg));
                break;
            case '?':
                printUsage(argv[0]);
                exit(-1);
            default:
                printUsage(argv[0]);
                exit(-1);
        }
    }

    int i=0;
    for(i=0; blinks>i; i++){
        writeToFile(&LIGHT_ON);
        //printf("on\n");
        usleep(timeout);
        writeToFile(&LIGHT_OFF);
        //printf("off\n");
        usleep(timeout);
    }
    //fclose(filePtr);
}
