#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <wait.h>
#define MAX_LENGTH 1024
/*****************************Global Variables**********************************************************/
int commandRecevied,insertSpace,fileOperationInputFlag,fileOperationOutputFlag,numberOfArguments,numberOfChildProcess=0,no_reprint_prmpt=0;
pid_t childProcessId[MAX_LENGTH];
/*****************************End of Global Variables**********************************************************/
/*****************************Function Declarations**********************************************************/
/*****************************Function Declarations**********************************************************/
int pipeSeparatedCommandParser(char line[],char *listOfPipeSeparatedCommands[],int numberOfPipeSeparatedCommands);
int checkForBackgroundProcess(char command[]);
void trim(char listOfCommands[]);
void tokenParser(char line[],char shellCommand[],char argumentsArray[],char inputFileName[],char outputFileName[]);
void append(char *charArray,char charToAppend);
int createListOfArguments(char *listOfArguments[],int numberOfArguments,char argumentsArray[]);
void pidToString(pid_t a, char b[]);
/***************************************************************************************/
/*****************************Main Function*****************************/
int main(int argc, char *argv[]) 
{
while(1)
		{	
		printf("minish>");
		char line[MAX_LENGTH]="";
		int i,j,k,backgroundProcess=0;
		pid_t pid;
		FILE *inputFp,*outputFp;
		if (!fgets(line, MAX_LENGTH, stdin)) break;
		char *listOfPipeSeparatedCommands[MAX_LENGTH]={NULL};
		int childstatus,numberOfPipeSeparatedCommands = 0;
		numberOfPipeSeparatedCommands = pipeSeparatedCommandParser(line,listOfPipeSeparatedCommands,numberOfPipeSeparatedCommands);// Caluclate and store the total number of pipe separated command
		backgroundProcess = checkForBackgroundProcess(listOfPipeSeparatedCommands[numberOfPipeSeparatedCommands-1]);// check for bacground process
		for(j=0;j<=numberOfPipeSeparatedCommands-1;j++)
		{
			trim(listOfPipeSeparatedCommands[j]);//trim first and last space of every command and also replace \n from the command with \0
		}
		//Executing one by one Pipe Separated command
		for(j=0;j<=numberOfPipeSeparatedCommands-1;j++)
		{ 
			char *listOfArguments[MAX_LENGTH]={NULL};
			numberOfArguments=0;
			char mainCommand[MAX_LENGTH]  = "/bin/";
			char shellCommand[MAX_LENGTH]="";
			char currentWorkingDirectory[MAX_LENGTH]="";
			char argumentsArray[MAX_LENGTH]="";
			char inputFileName[MAX_LENGTH]="";
			char outputFileName[MAX_LENGTH]="";
			commandRecevied =0;
			insertSpace=0;
			int kl=0;
			fileOperationInputFlag =0;
			fileOperationOutputFlag =0;
			strcpy(line,listOfPipeSeparatedCommands[j]);
			tokenParser(line,shellCommand,argumentsArray,inputFileName,outputFileName);//Token Parser which fetches all the arguments from the respective commands
			trim(argumentsArray);

			if(argumentsArray[0] != '\0')			
				numberOfArguments =	createListOfArguments(listOfArguments,numberOfArguments,argumentsArray);// Create a list of Arguments which needs to be Passed to execv
			
			if(fileOperationInputFlag == 1)
				inputFp = fopen(inputFileName,"r");
			if (fileOperationOutputFlag ==1)
				outputFp = fopen(outputFileName,"w");
/****************************************************Handles Signals****************************************************/
			if(strcmp(shellCommand,"exit")==0)
			{
					exit(0);
			}
/*******************************************cd Command*****************************************************************/
			else if(strcmp(shellCommand,"cd")==0 && strcmp(argumentsArray,"/")!=0)//process to handle cd command
			{
				getcwd(currentWorkingDirectory,sizeof(currentWorkingDirectory));
				if(currentWorkingDirectory[strlen(currentWorkingDirectory)-1]!='/')
					strcat(currentWorkingDirectory,"/");
				strcat(currentWorkingDirectory,argumentsArray);
				chdir(currentWorkingDirectory);
			}
			else if(strcmp(argumentsArray,"/")==0)
			{	
				chdir(argumentsArray);
			}
/*******************************************End of cd Command Handling*****************************************************************/
			
/*******************************************fg Command*****************************************************************/
			else if(strcmp(shellCommand,"fg" )==0)
			{
				if(numberOfArguments>0)
				{
					int i=0;
					int processNumber = atoi(listOfArguments[0]);
					if(childProcessId[processNumber]>0)
					{
						waitpid(childProcessId[processNumber], &childstatus, 0);
						if(childstatus==0)
							{
								printf("[%d] %d                Done\n",processNumber,childProcessId[processNumber]);
								childProcessId[processNumber] = -1;
							}
					}
				}
				else
				{
					for(i=0;i<=numberOfChildProcess-1;i++)	
					{
						waitpid(childProcessId[i], &childstatus, 0);  
						if(childstatus==0)
						{
							childProcessId[i] = -1;
						}
					}							
				}
			}
/*******************************************End of fg Command Handling*****************************************************************/
/*******************************************kill Command Handling******************************************************************/
			else if(strcmp(shellCommand,"kill")==0)
			{
				for(i=0;i<numberOfChildProcess;i++)
				{
					if(childProcessId[i]>0)
						kill(childProcessId[i],SIGKILL);
				}
			}
/*******************************************End of kill Command Handling*****************************************************************/
			else
			{
				strcat(mainCommand,shellCommand);
				char *command[numberOfArguments+2];
				int noOfCommand=0;
				command[noOfCommand] = shellCommand;
				noOfCommand++;
				if(argumentsArray[0]!='\0')
					{	
						for(i=1;i<=numberOfArguments;i++)
							if(strcmp(listOfArguments[i-1],"")!=0)
								{
									command[noOfCommand]=listOfArguments[i-1];
									noOfCommand++;
								}
						command[noOfCommand] = NULL;
					}
				else
						command[1] = NULL;
/*****************************************End of Reading the Command****************************************************************/
/*****************************************Single Commands****************************************************************/
				if(numberOfPipeSeparatedCommands==1)
				{
					if(strcmp(shellCommand,"")==0) //if enter is pressed, do nothing
					{
						// printf(" entred enter\n");
					}	
					else
					{
						pid = fork();
						if (pid == 0) //start of child code
						{
							if(fileOperationInputFlag==1)
							{
								dup2(fileno(inputFp),fileno(stdin));		
							 //	fclose(fp);
							}

							if(fileOperationOutputFlag==1)
								{
									dup2(fileno(outputFp),fileno(stdout));		
									//fclose(fp);
								}
							if(fileOperationInputFlag==1)	
								fclose(inputFp);
							if(fileOperationOutputFlag==1)
								fclose(outputFp);
							execv(mainCommand,command);
						}//End of Child code
						if (pid>0) //start of Parent Process
						{
								if(backgroundProcess==0)
								{
									childProcessId[numberOfChildProcess]= pid;
									waitpid(childProcessId[numberOfChildProcess], &childstatus, 0);
									if(childstatus!=0)
									{
										numberOfChildProcess++;
									}												
								}// end of background process check
								else
								{
									//Code to Handle Background Process
									childProcessId[numberOfChildProcess]= pid;
									printf("[%d] %d\n",numberOfChildProcess,childProcessId[numberOfChildProcess]);
									numberOfChildProcess++;
									waitpid(-1, NULL, WNOHANG);
								}
						}//end of parent process
					}
				}//End of if for one comand			
				else 
					{ //Code to manage PIPE
						int temp_iter = j;
						int fileDesciptors1[2]; // for odd  j
						int fileDesciptors2[2];	// for even j	
						if ((temp_iter % 2) != 0)
						{
							pipe(fileDesciptors1); // for odd i
						}else
						{
							pipe(fileDesciptors2); // for even i
						}		
						pid = fork();
						if(pid==0)
						{
							if (temp_iter == 0)
							{	//printf("j==0 \n");
								dup2(fileDesciptors2[1], STDOUT_FILENO);
							}
							else if(temp_iter==numberOfPipeSeparatedCommands-1) 
							{
								if((numberOfPipeSeparatedCommands/2)*2==numberOfPipeSeparatedCommands)
								{
									dup2(fileDesciptors2[0],STDIN_FILENO);
									close(fileDesciptors2[0]);
								}
								else
								{
									dup2(fileDesciptors1[0],STDIN_FILENO);
									close(fileDesciptors1[0]);
								}
							}//end of j==numberOfPipeSeparatedCommands
							else 
							{
								if((temp_iter/2)*2==temp_iter)
								{
									dup2(fileDesciptors1[0],STDIN_FILENO);
									dup2(fileDesciptors2[1],STDOUT_FILENO);
									close(fileDesciptors1[0]);
									close(fileDesciptors2[1]);
								}
								else//in odd else ====fileDesciptors2[0]
								{
									dup2(fileDesciptors2[0],STDIN_FILENO);
									dup2(fileDesciptors1[1],STDOUT_FILENO);	
									close(fileDesciptors2[0]);
									close(fileDesciptors1[1]);
								}
							}//end of j!=numberOfPipeSeparatedCommands
								if(fileOperationInputFlag==1)
								{
									dup2(fileno(inputFp),fileno(stdin));		
								}

							if(fileOperationOutputFlag==1)
								{
									dup2(fileno(outputFp),fileno(stdout));		
								}
						if(fileOperationOutputFlag==1)	
							fclose(inputFp);
						if(fileOperationInputFlag==1)
							fclose(outputFp);
							execv(mainCommand,command);
							perror("execv");
							exit(2);
						}
								
							if (temp_iter == 0) //close command filedes2[1]
								{
									close(fileDesciptors2[1]);
								}
								else if (temp_iter==numberOfPipeSeparatedCommands-1)//last even close command filedes2[0]
								{
									if ((numberOfPipeSeparatedCommands/2)*2==numberOfPipeSeparatedCommands)
									{					
										close(fileDesciptors2[0]);
									}
									else
									{					
										close(fileDesciptors1[0]);
									}
								}
								else
								{
									if ((temp_iter/2)*2==temp_iter)
									{					
										close(fileDesciptors1[0]);
										close(fileDesciptors2[1]);
									}
									else
									{					
										close(fileDesciptors2[0]);
										close(fileDesciptors1[1]);
									}
								}
						//	waitpid(pid,NULL,0);
						//end of pid > 0
						
						if(backgroundProcess==0)
							{
									childProcessId[numberOfChildProcess]= pid;
									waitpid(childProcessId[numberOfChildProcess], &childstatus, 0);
									if(childstatus!=0)
									{
										numberOfChildProcess++;
									}
							}// end of background process check
						else
							{
								//Code to Handle Background Process
								childProcessId[numberOfChildProcess]= pid;
								numberOfChildProcess++;
								waitpid(-1, NULL, WNOHANG);
							}
					}//End of else for more than one comand			
			}//End of else handle other commands
		} //End of For loop for Pipe Separated Commands
}// End of While loop
	return 0;
}
/*****************************End of Main**********************************************************/



/*****************************Create a list of Arguments**********************************************************/
int createListOfArguments(char *listOfArguments[],int numberOfArguments,char argumentsArray[])
{
	listOfArguments[numberOfArguments] = malloc(sizeof(listOfArguments[numberOfArguments]));
	char temp[MAX_LENGTH]="";
	int i;
	for(i=0;i<=strlen(argumentsArray);i++)
	{	
		if(argumentsArray[i] == '\\')
			{
				append(temp,' ');
				i++;
			}
		else if(argumentsArray[i]==' ' || argumentsArray[i]=='\0')
			{
				append(temp,'\0');
				char *t = temp;
				strcpy(listOfArguments[numberOfArguments],temp);
				numberOfArguments++;
				temp[0]='\0';
				listOfArguments[numberOfArguments] = malloc(sizeof(listOfArguments[numberOfArguments]));
			}
		else
			append(temp,argumentsArray[i]);
	}
	return numberOfArguments;
}
/*****************************End of Create a list of Arguments**********************************************************/
/*****************************Token Parser which Capture shell command the arguments and list of Arguments adn fileName**********************************************************/
void tokenParser(char line[],char shellCommand[],char argumentsArray[],char inputFileName[],char outputFileName[])
{
	int i,inputFileNameCaptured=0,outputFileNameCaptured=0;
	for(i=0;line[i]!=0;i++)	
	{
		if(line[i]==' ')		
			{
				commandRecevied=1;	
				insertSpace++;
				if(insertSpace>1 && fileOperationInputFlag == 0 && fileOperationOutputFlag ==0 )		
					append(argumentsArray,line[i]);
			}
		else if(commandRecevied==0 && line[i]!='\0')
			append(shellCommand,line[i]);
		else if(commandRecevied==1 && line[i]=='<')
			{fileOperationInputFlag = 1;outputFileNameCaptured=1;inputFileNameCaptured=0;}
		else if(commandRecevied==1 && line[i]=='>')
			{fileOperationOutputFlag = 1;inputFileNameCaptured=1;outputFileNameCaptured=0;}		
		else if(commandRecevied==1 && line[i]!='\0' && fileOperationOutputFlag==0 && fileOperationInputFlag==0)
			append(argumentsArray,line[i]);
		else if(fileOperationInputFlag == 1 && inputFileNameCaptured==0  && line[i]!='\0')
			append(inputFileName,line[i]);
		else if(fileOperationOutputFlag ==1 && outputFileNameCaptured==0 && line[i]!='\0')
			append(outputFileName,line[i]);
		}


append(argumentsArray,'\0');
}
/*****************************Trime the first and last space**********************************************************/
void trim(char listOfCommands[])
{
	char temp[MAX_LENGTH]="";
	int k,fistSpaceFlag=0;
		strcpy(temp,listOfCommands);
		if(temp[0]==' ')
		{
			for(k=1;k<=strlen(temp)-1;k++)
				temp[k-1] =temp[k];
			fistSpaceFlag = 1; 
		}
		if(temp[strlen(temp)-1]==' ' || temp[strlen(temp)-1]=='\n')
		{
			if(fistSpaceFlag == 1)
				temp[strlen(temp)-2]='\0';	
			else
				temp[strlen(temp)-1]='\0';
		}
		strcpy(listOfCommands,temp);
}
/*****************************End of Trime the first and last space**********************************************************/
/*****************************Check Background Process Commands**********************************************************/
int checkForBackgroundProcess(char command[])
{
	char temp[MAX_LENGTH]="";
	int backgroundProcess = 0;
	strcpy(temp,command);
	if(temp[strlen(temp)-2]=='&')
	{
		temp[strlen(temp)-2]='\0';
		backgroundProcess =1;
	}
	strcpy(command,temp);
return backgroundProcess;
}
/*****************************End of Check Background Process Commands**********************************************************/
/*****************************Pipe Separated Commands**********************************************************/
int pipeSeparatedCommandParser(char line[],char *listOfPipeSeparatedCommands[],int numberOfPipeSeparatedCommands)
{
	char *temp = strtok (line,"|");
	while (temp != NULL) 
	{
		listOfPipeSeparatedCommands[numberOfPipeSeparatedCommands] = temp;
		temp = strtok(NULL, "|");
		numberOfPipeSeparatedCommands++;
  	}
	listOfPipeSeparatedCommands[numberOfPipeSeparatedCommands] = NULL;
	return numberOfPipeSeparatedCommands;
}
/*****************************End of Parser for Pipe Separated Commands**********************************************************/
/******************************Append single character to the character array************************************************************/
void append(char *charArray,char charToAppend)
{
	int charArrayLength = strlen(charArray);
	charArray[charArrayLength]= charToAppend;
	charArray[charArrayLength+1]='\0';
}
/*****************************End of Append**********************************************************/
void pidToString(pid_t a, char b[])
{
//	pid_t a = 5598;
	char t[10]="";
	int i=0;
	while(a!=0)
	{
		int digit = a%10;
		t[i] = digit + '0';
		a=a/10;
		i++;
	}

	t[i]='\0';

//	char b[i+1];
	int j;
	for(j=0;j<=i-1;j++)
		b[j] = t[i-j-1];
	b[j]='\0';
//	printf("String %s %s %d\n",t,b,a);
}