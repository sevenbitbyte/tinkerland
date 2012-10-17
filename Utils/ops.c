#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define DATASIZE 100
void Alarm_Handle();
void report();  //This is a prototype for a function that is called when Ctrl+C is pressed
void help();  //Prototype for the logo function

int numpart1, numpart2, numpart3, numpart4, stop1, stop2, stop3, stop4, error, port=0;
int scanned, open, forked, time=0;  //Hold info on how many host were scanned, responded, and how many precesses were forked.
int timeout=1;  //Default connection timeout. This is howlong we try to contact a host.
int Am_I_A_Child=0;

int main(int argc, char *argv[]){
  (void) signal(SIGINT,report);  //Sets a signal handler for the Ctrl+C interrupt. This does not actually stop the scan.
  (void) signal(SIGALRM, Alarm_Handle); 
  alarm(1);
    if(argc < 3){//Check if 2 IPs and a port# were entered
      help(argv);
      return 1;
    }
    
    error=num_subnet(argv[3]); //Parse the end IP out.
    if(error!=0){  //Check for errors
      printf("An error of code(%i.1) occured!\n", error); //Report an error occured
      help(argv);
      printf("An error of code(%i.1) occured!\n", error); //Report an error occured
      exit(-1);  //Exit
    }
    stop1=numpart1; //Save the 1st number from the end IP
    stop2=numpart2; //Save the 2nd number from the end IP
    stop3=numpart3; //Save the 3rd number from the end IP
    stop4=numpart4; //Save the 4th number from the end IP
    port=atoi(argv[2]); //Convert the port number to an integer from a character
if(port < 0 || port > 63300){printf("%i is an invalid port.\n", port);return -1;}  //Determine if the port number is allowed
    error=num_subnet(argv[1]); //Parse the start IP out.
    if(error!=0){ //Check for errors
      printf("An error of code(%i.2) occured!\n", error); //Report an error occured
      help(argv);
      exit(-1);  //Exit
    }
    possible(); //Generate and test all possible IPs
    printf("%i host scanned on port %i in %iseconds. %i host found open.\nI forked a total of %i processes.\n", scanned, port, time, open, forked);  //Give a summary of open and closed host.
    return; //Return
}

void report(int sig){}

void Alarm_Handle(){
time++;
if(Am_I_A_Child==1 && time == timeout){exit(9);}
alarm(1);
}

int try(char address[]){
  (void) signal(SIGALRM, Alarm_Handle); 
  alarm(1);
  int sock;
  struct hostent *h;
  struct sockaddr_in their_addr;

  if ((h=gethostbyname(address)) == NULL){
    return 2;
  }
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    return 3;
  }

  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(port);
  their_addr.sin_addr = *((struct in_addr *)h->h_addr);

  if (connect(sock, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1){
  return 4;
  }
  close(sock);
  return 0;
}

int possible(){
  pid_t   pid, child_pid, sub_child; //Process ids
  int cnumpart1=numpart1;
  int cnumpart2=numpart2;
  int cnumpart3=numpart3;
  int cnumpart4=numpart4;
  int fork_error, fork_killed=0;
  int status; //Status of the child process when it exits
  char address[17];
  
  while(cnumpart1 <= stop1){
    while(cnumpart2 <= stop2){
      while(cnumpart3 <= stop3){
	while(cnumpart4 <= stop4){
	  sprintf(address, "%i.%i.%i.%i", cnumpart1, cnumpart2, cnumpart3, cnumpart4);
	    pid=fork(); //Starts a child process
	    if(pid < 0){  //This could mean that there are no more system reasources.
	      fork_error++;
	      printf("Forking Error(#%i)...Sleeping\n", fork_error);
	      sleep(10);
	    }
            //This is where the first child process starts.
//	    if(pid == 0){
//	      child_pid=fork();
child_pid=pid;
              //Here is the sub-child process
	      if(child_pid == 0){
	      Am_I_A_Child=1;
	        if(try(address)==0){printf("%s\n", address);exit(0);}
	        exit(9);  //The sub-child ends here.
	      }

//	    sleep(timeout);
//	    kill(child_pid, 9);
//            wait(&status);
//              if(status==0){exit(0);}
//	    exit(9);
//	    }
	  cnumpart4++;
	  scanned++;	
          fork_error=0;
	}
	cnumpart3++;
	cnumpart4=numpart4;
      }
      cnumpart2++;
      cnumpart3=numpart3;
    }
    cnumpart1++;
    cnumpart2=numpart2;
  }

  while(forked!=scanned){
    forked++;
    wait(&status);
    if(status!=0){fork_killed++;} //Counts how many process returned non 0, meaning they timedout.
    status=0;  //Reset the status variable so we don't get bad info.
  }
//forked=forked*2;
open=(scanned-fork_killed);  //Calculate how many host were open by 
}

int num_subnet(char *argv1){
  if(!argv1){
    return 1;
  }
  int length = strlen(argv1); //Find the length of the IP string
  char temp[1];
  char period[1]=".";
  char subnet_start[length]; //Create a char to hold the IP string
  int i=0;
  int point1=0;
  int point2=0;
  int point3=0;

  if(length > 15){return 1;} //Check to see if the IP address is too long.
  if(length < 7){return 2;}  //Check to see if the IP address is too short.

  if(sprintf(subnet_start, "%s", argv1)){ //Copy the IP into the subnet_start and error check
  }
  else{
    printf("There appears to have been an error copying the string.\n"); //The string could not be copied
    return 2;  //Notify the user
  }

  while(i < length){ //Find out where the periods are in the IP string
  temp[0]=subnet_start[i];  //Set a character to look at
    if(strncmp(temp, period, 1)==0){
      if(point1==0){ //The first period has been found
	point1=i;  //Set the location of the 1st period
      }
      else if(point2==0){ //The 2nd period was found
	point2=i;  //Set the location of the 2nd period
      }
      else if(point3==0){ //The 3rd period was found
	point3=i;  //Set the location of the 3rd period
      }
    }
    bzero(temp, 1); //Clear the temp char so that it can be used again
    i++;
  }

  if(point1==0 || point2==0 || point3==0){ //Make sure that the IP contained 3 periods
    printf("There appear to be missing periods in the supplied IP addess.\n");
    return 3;
}
  i=0;  //Reset i for later use.

  //*******************************************
  //The following splits the IP address into
  //four parts based on the location of the
  //periods. Then the parts are converted from
  //characters to integers.
  //*******************************************
  int part1len=point1;
  int part2len=((point2-point1)-1);
  int part3len=((point3-point2)-1);
  int part4len=((length-point3)-1);
  char part1[part1len];
  char part2[part2len];
  char part3[part3len];
  char part4[part4len];
  //This while separates out the numbers from the subnet string
  while(i != length){
    if(i <= part1len){strncpy(&part1[i], &subnet_start[i], 1);}
    if(i <= part2len){strncpy(&part2[i], &subnet_start[point1+i+1], 1);}
    if(i <= part3len){strncpy(&part3[i], &subnet_start[point2+i+1], 1);}
    if(i <= part4len){strncpy(&part4[i], &subnet_start[point3+i+1], 1);}
    i++;
    }
  if(!atoi(part1) || !atoi(part2) || !atoi(part3) || !atoi(part4)){return -3;} //Make sure that the numbers are actually numbers and can be converted.
  //The following converts the string values into numbers.
  numpart1=atoi(part1);
  numpart2=atoi(part2);
  numpart3=atoi(part3);
  numpart4=atoi(part4);
  if(numpart1 > 255 || numpart2 > 255 || numpart3 > 255 || numpart4 > 255){return -4;} //Make sure the numbers are not larger than 255
  if(numpart1 < 0 || numpart2 < 0 || numpart3 < 0 || numpart4 < 0){return -5;} //Make sure the numbers are not less than 0
  return 0;  //Return 0 meaning there were no errors
}

int help(){
//print a cool NULL logo
printf("    ______     ___  ___  ___  ___  ___");
printf("\n");
printf("   /      \\   /  / /  / /  / /  / /  /");
printf("\n");
printf("  /   /\\   \\ /  / /  / /  / /  / /  /");
printf("\n");
printf(" /   /  \\   /  / /  /_/  / /  / /  /");
printf("\n");
printf("/___/    \\____/  \\______/ /__/ /__/");
printf("\n");

     // printf("\nUsage: %s <start IP> <port> <end IP> [options]\n", argv[0]);
      printf("\n\tThis server scanner is still in testing phase. It is a\n");
      printf("multi-threaded program allowing it to scan many ip addresses\n");
      printf("simotanously.\n");
      printf("\t-t <microseconds>    Sets the timeout\n");
      printf("\t-s <microseconds>    Sets the scan speed\n");
      printf("\t-m                   Turns off multi threading.\n");
      printf("\t-o <file             Redirects output to <file>\n");
      printf("\t-e                   Turns off error messages\n");
      printf("\t-r <tries>           Sets number of times to re-try if forking fails\n");
      printf("Common problems\n");
      printf("\tHow should I know LMAO.. Oh yea, none of the above options work yet.\n");
return 0;  //Return
}
