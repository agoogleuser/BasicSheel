#include <iostream>     //Terminal I/O
#include <fstream>      //file I/O
#include <unistd.h>     //fork(), execvp() system call s
#include <string.h>     //String Class
#include <sys/wait.h>   //waitpid() system call
#include <sys/types.h>  //pid_t variable
using namespace std;

//Global Variables
pid_t process_id;
int process_status=0;
short process_counter = 0;
ofstream logFile(".log.txt");


char **stringArr_to_charArr(string input, int *words_num)
{
    /**
     * @brief Convert a C++ string into a an array of C strings
     * at the locations of spaces.
     * @param words_num is an output that holds the number of words. 
     */
    char **output = (char **)calloc(16, sizeof(char *));
    int i = 0, j = 0, pos = 0, size = input.length();

    // ->>input = "This is an example"
    for (i = 0; i <= size; i++)
    {
        if (input[i] == ' ' || input[i] == '\0')
        {
            output[j] = (char *)malloc((i - pos) * sizeof(char));
            strcpy(output[j], input.substr(pos, i - pos).c_str());
            //debugString.insert(0, input.substr(pos, i - pos));
            pos = i + 1;
            j++;
        }
    }
    *words_num = j;
    // ->>output={"this", "is", "an", "example", NULL}
    return output;
}

void handler(int num)
{//Function that automaticaly operates when a child process dies.
    pid_t childPID = waitpid(-1, &process_status, WNOHANG);
    process_counter--;
    logFile <<"| Child [" << childPID <<"] Finished Execution. \n\n";
    //Saves this output in .log.txt file, which is hidden by default for linux.
}

string remove_extra_spaces(string input)
{
    //Remove spaces at the beginning
    while (input[0]==' ')
        input.erase(0, 1);
    
    //remove spacesin the middle
    for(int i=1;input[i-1];i++)
        if (input[i]==' ' && input[i+1]==' ')
            input.erase(i--,1);

    //remove spaces at the end
    while (input.back()==' ')
        input.erase(input.length()-1, 1);

    return input;
}

int main()
{
    bool check;
    int wordSize;       //Stores the Number of arguments in each input
    string Line_Input;  //User Input buffer
    string command;     //First Input Argument
    char currentDirectory[0xFF]; //Stores the current directory in a C string
    char userName[0x20];//getlogin_r(userName, 0x20);
    cuserid(userName);
    char **commandArguments = NULL; //Stores the user input to use in execvp();

    signal (SIGCHLD, handler);
    cout << "Starting our basic shell:\n++++++++++++++++++++++++++\n";

    while (1)
    {
        //Initializing variables every loop.
        check = false;
        wordSize=0;
        Line_Input = "";
        command = "";
     
        getcwd(currentDirectory, 0xFF); //Calls a system function to get the current working directory.
        cout << endl << userName << ':' << currentDirectory << "$ "; 

        // 0.  Getting Input Buffer from the user in the terminal.
        getline(cin, Line_Input);
        // 0.1 Remove extra spaces from the input
        Line_Input = remove_extra_spaces(Line_Input); 
        // 0.2 Stores The Line input (after fixing it) into a log file
        logFile << "command: \"" << Line_Input <<"\"\n";
        // 0.3 check if there was an '&' at the end of the line
        check = (Line_Input.back()=='&')?true:false;
        if (check==true){
            Line_Input.erase(Line_Input.find_last_of('&')); //removes '&'
            Line_Input.erase(Line_Input.end()-1);//removes extra space before '&'.
        }
       
        // 1.  Check if the input from user was "exit"
        if (Line_Input == "exit")   break;

        // 2.  Split the input buffer using the following lines

        command.assign(Line_Input.substr(0, Line_Input.find(" ")));
        // 2.1 If it was an 'sudo' command, Add "-S" in front of it.
        // This is because it requres a standard input for some reason.
        if (command == "sudo")
            Line_Input.insert(4," -S");

        commandArguments = stringArr_to_charArr(Line_Input, &wordSize);

        // 3.  Check if the input was "cd", It will change the working directory
        //     for the parent process.
        if (command == "cd")
            chdir(*((commandArguments) + 1));

        else
        {//4.  Executing General commands
            process_id = fork();
            process_counter++; //Assuming new fork was created successfully;
            if (process_id == 0)
            //Child Process goes here.
                execvp(command.c_str(), commandArguments);
            
            else if (process_id > 0)
            {//Parent process goes here
                if (check==false)                
                    waitpid(process_id,&process_status ,0);
                    //Wait for the last child to die. (don't take this comment literally pls)
                else
                    logFile << '[' << process_id << "] + " << process_counter << "\tis Working In Background.\n";
            }
            else
            {//In case a process was not created.
                process_counter--;
                printf("A new process can't be created. Please try again.\nIf the issue still exists please free some memory from your system");
            }
        }
        //5.  Free commandArgumentf from memory every loop.
        for (int i = 0; i < wordSize; i++)
            free(*(commandArguments + i));

        free(commandArguments);

        //If something wrong happens, Force quit the program.
        if (process_counter >= 10) break;       
    }
    cout << "Exiting...\n+++++++++++++++++++++++++++++++++++++++++++++\n";
    logFile.close();
    kill(0, SIGKILL); //Kills all children of the main process.
    return 0;
}