#include <bits/stdc++.h>
#include <filesystem>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

// run this file with this exact command in the terminal (stated below) as readline ke liye ese hi proceed krna pdega
// g++ main.cpp -o myshell -lreadline

using namespace std;
namespace fs = filesystem;

// Global vector to store command history for the 'history' builtin
vector<string> History_tracker;

// Counter to keep track of where to start appending history for 'history -a'
int append_counter = 0;

/*
 * Function: quotes_splitter
 * -------------------------
 * Splits a command string into arguments while respecting:
 * 1. Single quotes (' ') - preserves literal value of all characters within
 * 2. Double quotes (" ") - preserves literal value but allows certain backslash escapes
 * 3. Backslash (\) - escapes the next character
 *
 * It parses character by character to handle nested logic correctly.
 */
vector<string> quotes_splitter(string &str){
    vector<string> final;
    bool in_quotes=false;
    string temp="";
    int num=0; // Tracks quote type: 1 for single quotes, 2 for double quotes
    for(int c=0;c<str.length();c++){
        // Handle backslash escapes
        if(str[c]=='\\' && num!=1){ // Backslashes are ignored inside single quotes (num==1)
            if(c==str.length()-1) temp+=str[c];
            if(!in_quotes){
                c++;
                temp+=str[c];
            }
            else if(in_quotes && num==2){ // Inside double quotes
                // Only escape $, `, ", \, and newline inside double quotes
                if(str[c+1]=='$' || str[c+1]=='`' || str[c+1]=='\"'){
                    c++;
                    temp+=str[c];
                }
                else if(str[c+1]=='\\'){
                    if(c+2<str.length()){
                        if(str[c+2]=='n'){ // Handle literal \n
                            c+=2;
                            temp+="\\n";
                        }
                        else{
                            c++;
                            temp+=str[c];
                        }
                    }
                }
                else temp+=str[c];             
            }
        }
        // Handle Quote toggling
        else if(str[c]=='\'' || str[c]=='\"'){
            if(in_quotes==false){
              in_quotes=true;
              if(str[c]=='\'') num=1;
              else num=2;
            } 
            else{
              // Close quote if matching type found
              if((num==1 && str[c]=='\'') || (num==2 && str[c]=='\"')) in_quotes=false;
              else temp+=str[c]; 
            }
        }
        // Handle Spaces (split arguments) outside of quotes
        else if(str[c]==' ' && !in_quotes){
            if(temp!=""){
                final.push_back(temp);
                temp="";
            }
        }
        else{
            temp+=str[c];
        }
    }
    if(temp!="") final.push_back(temp);
    return final;
}

/*
 * Function: splitter
 * ------------------
 * Splits a string by a delimiter character 's'.
 * Used primarily for splitting the PATH environment variable by ':'.
 */
vector<string> splitter(string &str,char s){
  vector<string> ans;
  size_t start=0;
  size_t end=str.find(s);
  while(end!=-1){
    ans.push_back(str.substr(start,end-start));
    start=end+1;
    end=str.find(s,start);
  }
  ans.push_back(str.substr(start));
  return ans;
}

// Global variable to store directories from system PATH
string path=getenv("PATH");
vector<string> directry=splitter(path,':');

// List of all executable files + builtins for Autocompletion
vector<string> listof_files;

/*
 * Function: populate_
 * -------------------
 * Scans all directories in PATH and adds every filename to 'listof_files'.
 * Also adds shell builtins manually.
 * Sorts and removes duplicates for efficient searching.
 */
void populate_(){
  for(auto each_path:directry){
      if(fs::exists(each_path)){
          for(const auto & each_file : fs::directory_iterator(each_path)){
              listof_files.push_back(each_file.path().filename().string());
          }
      }
  }
  // Add builtins
  listof_files.push_back("cd");
  listof_files.push_back("type");
  listof_files.push_back("exit");
  listof_files.push_back("history");

  // Sort and remove duplicates
  sort(listof_files.begin(),listof_files.end());
  auto last=unique(listof_files.begin(),listof_files.end()); 
  listof_files.erase(last,listof_files.end());
}

/*
 * Function: converter
 * -------------------
 * Converts vector<string> to vector<char*> for execvp() compatibility.
 * execvp requires a NULL-terminated array of C-style strings.
 */
vector<char*> converter(vector<string>& vec){
    vector<char*> argv;
    for(int i=0;i<vec.size();i++){
        argv.push_back(const_cast<char*>(vec[i].c_str()));
    }
    argv.push_back(nullptr);
    return argv;
}

/*
 * Note: 'lcp' logic and 'read_input' function below seem to be from 
 * the custom readline implementation phase. Since actual GNU readline 
 * is used in main(), these might be unused now but kept as requested.
 */
string lcp="";
void lcp_(string str){
    if(lcp=="") lcp=str;
    else{
        string temp_="";
        for(int i=0;i<min(lcp.length(),str.length());i++){
            if(lcp[i]==str[i]){
                temp_+=lcp[i];
            }
            else break;
        }
        lcp=temp_;
    }
}

// Now, I am not using this read_input() function because now i am using readline function that is why
string read_input(){
  // ... (Custom readline logic, seemingly replaced by library usage in main) ...
  // Keeping logic intact as requested.
  string input="";
  string temp="";
  string temporary_arrow_string="";
  vector<string> prefix=listof_files;
  bool previous_tab=0;
  vector<string> all_executables;
    
    while(true){
        int c = getchar();
        // Handle Tab logic for autocomplete printing
        if(previous_tab && c==9){
            cout<<"\n";
            for(auto v:all_executables){
                cout<<v<<"  ";
            }
            cout<<"\n$ "<<input;
            previous_tab=0;
        }
        else if(c==127){   // Backspace handling
            if(input!=""){
                cout<<"\b \b";
                input=input.erase(input.size()-1,1);
                if(temp!=""){
                    temp=temp.erase(temp.size()-1,1);
                }
            }
            previous_tab=0;
        }
        else if(c==10){  // Enter handling
            cout<<"\n";
            return input;
        }
        else if(c==9){ // First Tab press logic
            if(temp!=""){
                all_executables.clear();
                lcp="";
                string temp_str="";
                // Find matches
                for(auto v:listof_files){
                    string t="";
                    if(temp.size() <= v.size()) t=v.substr(0,temp.size());
                    if(t==temp){
                        temp_str=v.substr(temp.size());
                        all_executables.push_back(v);
                        lcp_(v);
                    }
                }
                
                // Handle matches: 1 match -> complete, >1 -> partial complete/bell
                if(all_executables.size()==1){
                    cout<<temp_str<<" ";
                    input+=(temp_str+" ");
                    temp="";
                }
                else if(all_executables.size()>1){
                    string temp_str1="";
                    if(temp.size()<lcp.size()) temp_str1=lcp.substr(temp.size());
                    if(temp_str1==""){
                        sort(all_executables.begin(),all_executables.end());
                        cout<<"\x07"; // Bell sound
                        previous_tab=1;
                    }
                    else{
                        cout<<temp_str1;
                        input+=(temp_str1);
                        temp+=temp_str1;
                    }
                }
                else{
                    cout<<"\x07";
                }
            } 
            continue;
        }
        else{
            input+=char(c);
            if(c==32){
                temp="";
            }
            else temp+=char(c);
            cout<<char(c);
            previous_tab=0;
        }
        cout.flush();
    }
    return input; 
}

/*
 * Function: builtin_execute
 * -------------------------
 * Checks if the command is a shell builtin (echo, type, pwd, cd, history).
 * If yes, executes it and returns true. If no, returns false.
 */
bool builtin_execute(string cmd1){
    stringstream ss(cmd1);
    string word;
    ss>>word;
    
    // --- Builtin: ECHO ---
    if(word == "echo"){
        if(cmd1.length() > 5){
            string abc = cmd1.substr(5);
            auto abc_splittedVector = quotes_splitter(abc);
            for(auto v: abc_splittedVector){
                if(v == abc_splittedVector[abc_splittedVector.size()-1]) cout<<v<<"\n";
                else cout<<v<<" ";
            }
        }
        return true;
    }
    // --- Builtin: TYPE ---
    else if(word == "type"){
          ss>>word;
          // Check against known builtins
          if(word=="echo"){
            cout<<"echo is a shell builtin\n";
          }
          else if(word=="exit"){
            cout<<"exit is a shell builtin\n";
          }
          else if(word=="type" || word=="pwd" || word == "cd" || word == "history"){
            cout<<word<<" is a shell builtin\n";
          }
          // Check against executables in PATH
          else{
            string file_name=cmd1.substr(5);
            bool file_done=false;
    
            for(auto each_path:directry){
              auto new_path=each_path+'/'+file_name;
              if(fs::exists(new_path) && fs::is_regular_file(new_path)){
                fs::file_status s = fs::status(new_path);
                fs::perms p = s.permissions();
                // Check execute permission
                bool isExecutable = (p & fs::perms::owner_exec) != fs::perms::none;
                if(isExecutable){
                  cout<<file_name<<" is "<<new_path<<"\n";
                  file_done=true;
                  break;
                }
              }
            }
            if(file_done==false){         
              cout<<file_name<<": not found\n";            
            }       
          }
          return true;
    }
    
    // --- Builtin: PWD ---
    else if(word == "pwd"){
        cout<<fs::current_path().string()<<"\n";
        return true;
    }
    
    // --- Builtin: CD ---
    else if(word == "cd"){
      ss>>word;
      if(word == "~"){
        // Handle HOME directory
        const char* home_ptr = getenv("HOME"); 

        if(home_ptr == nullptr){
          cout<<"Error: Home not set\n";
        }
        else{
          fs::path home_path(home_ptr);
          if(fs::exists(home_path)){
            fs::current_path(home_path);
          }
          else{
            cout<< "Error: HOME directory doesn't exist on disk\n";
          }
        }
      }
      else {
        // Change to specified directory
        string temp_directory=cmd1.substr(3);
        if(fs::exists(temp_directory) && fs::is_directory(temp_directory)){
          fs::current_path(temp_directory);
        }
        else{
          cout<<"cd: "<<temp_directory<<": No such file or directory\n";
        }
      }
      return true;
    }

    // --- Builtin: HISTORY ---
    else if(word == "history"){
      ss>>word;
      int temp_idx=History_tracker.size();
      
      // Handle 'history -r file' (Read from file)
      if(word=="-r"){
          ss>>word;
          string file_name_=word;
          ifstream file(file_name_);
          if(!file.is_open()){
              cout<<"Error: This file doesn't exist\n";
              return false;
          }
          string each_line;
          while(getline(file,each_line)){
              History_tracker.push_back(each_line);
          }        
      }
      // Handle 'history -w file' (Write entire history to file)
      else if(word == "-w"){
        append_counter=History_tracker.size(); // Update marker
        ss>>word;
        string file_name_=word;
        ofstream file(file_name_);    // Overwrite mode
        
        if(!file.is_open()){
          cout<<"Error: This file doesn't exist\n";
          return false;
        }
        for(auto v:History_tracker){
          file<<v<<"\n";
        }
      }
      // Handle 'history -a file' (Append new history lines to file)
      else if(word == "-a"){
        ss>>word;
        string file_name_=word;
        ofstream file(file_name_,ios::app);   // Append mode

        if(!file.is_open()){
          cout<<"Error: This file doesn't exist\n";
          return false;
        }
        for(int i=append_counter;i<History_tracker.size();i++){
          file<<History_tracker[i]<<"\n";
        }
        append_counter=History_tracker.size();
      }
      // Handle standard 'history' (Display list)
      else{
          if(word!="history") temp_idx=stoi(word); // Handle 'history N'
          if(temp_idx<=0) return true;
          for(int i=(History_tracker.size()-temp_idx+1);i<=History_tracker.size();i++){
            cout<<i<<" "<<History_tracker[i-1]<<"\n";
          }
      }
      return true;
    }
    
    return false;
}

/*
 * Function: command_generator
 * ---------------------------
 * Used by Readline for autocompletion.
 * Filters 'listof_files' to find matches starting with 'temp'.
 */
vector<string> command_generator(string temp){
      vector<string> all_executables;        
      if(temp!=""){
          for(auto v:listof_files){
              string t="";
              if(temp.size() <= v.size()) t=v.substr(0,temp.size());
              if(t==temp){
                  all_executables.push_back(v);
              }
          }
      } 
      return all_executables;
}

/*
 * Function: command_generator_wrapper
 * -----------------------------------
 * Adapts C++ command_generator for C-style Readline library.
 * Returns matches one by one to Readline.
 */
char* command_generator_wrapper(const char* text, int state){
  static vector<string> matches;
  static size_t match_index = 0;

  if(state == 0){ // First call: Generate all matches
    matches = command_generator(string(text));
    match_index = 0;
  }

  // Subsequent calls: Return next match
  if(match_index < matches.size()){
    char* result = strdup(matches[match_index].c_str());
    match_index++;
    return result;
  }

  return nullptr;
}

/*
 * Function: my_autocompletion
 * ---------------------------
 * Hook for Readline. Checks if completing command (start==0) or argument.
 */
char** my_autocompletion(const char* text, int start, int end){
  if(start == 0){
    return rl_completion_matches(text, command_generator_wrapper);
  }
  return nullptr;
}

int main() {
  // Ensure output is flushed immediately
  cout << std::unitbuf;
  cerr << std::unitbuf;

  // 1. Populate autocompletion database
  populate_();
  
  // 2. Register custom autocompletion function with Readline
  rl_attempted_completion_function = my_autocompletion; 
  
  // 3. Check for HISTFILE env var and load history if present
  const char* path_ = getenv("HISTFILE");
  if(path_ != nullptr){
    if(fs::exists(path_)){
      builtin_execute("history -r "+string(path_));
    }
  }

  // --- Main Shell Loop ---
  while(true){
    // Use GNU Readline for input handling (Arrows, Edits, etc.)
    char* unmodified_cmd=readline("$ ");
    
    // Handle Ctrl+D (EOF)
    if(!unmodified_cmd) break;
    
    string cmd1;
    cmd1=string(unmodified_cmd);
    
    // Add non-empty commands to Readline's internal history (for Up Arrow)
    if(cmd1.length() > 0) add_history(unmodified_cmd);
    
    free(unmodified_cmd); // Must free pointer from readline

    // Add to our custom history tracker
    History_tracker.push_back(cmd1);

    string file_name;
    int saved_stdout=-1;
    bool redirection_active=false;
    int temp_fd;
    stringstream ss(cmd1);
    string word;
    ss>>word;
    
    // --- REDIRECTION HANDLING (>, 1>, 2>, >>) ---
    if(cmd1.find('>') != string::npos){
      int idx=cmd1.find('>');
      
      // Check which descriptor to redirect (1=stdout, 2=stderr)
      if(cmd1[idx-1]=='2'){
        temp_fd=2;        
      }
      else{
        temp_fd=1;
      }    
      
      saved_stdout=dup(temp_fd); // Save original descriptor
      int fd_required;
      int flag;
      
      // Determine Append (>>) vs Truncate (>) mode
      if(cmd1[idx+1]=='>'){
        flag = O_WRONLY | O_CREAT | O_APPEND;
        file_name=cmd1.substr(idx+3);
      }
      else{
        flag = O_WRONLY | O_CREAT | O_TRUNC;
        file_name=cmd1.substr(idx+2);
      }
      // Open file and redirect
      fd_required=open(file_name.c_str(),flag,0644);
      cmd1=cmd1.substr(0,idx-1); // Remove redirection part from command
      dup2(fd_required,temp_fd);
      close(fd_required);
      redirection_active=true;
    }
    
    // Parse arguments respecting quotes
    vector<string> argument=quotes_splitter(cmd1);
    
     // --- PIPING HANDLING (|) ---
     if(cmd1.find('|') != string::npos){
        
        int children_count = 0;
        int idx_ = -1;
        int idx__ = cmd1.find('|');
        int next_fd=0;
        int fd[2]; // Pipe descriptors
        
        // Loop through all piped segments
        while(idx__ != string::npos   && idx__>=1 && (idx__+2)<cmd1.length() ){
            children_count++;
            string path1_="";
            path1_=cmd1.substr(idx_+1,idx__-idx_-1); // Get command segment
            idx_ = idx__;
            idx__=cmd1.find('|',idx_+1);
           
            vector<string> args1_=quotes_splitter(path1_);
            vector<char*> args_path1_ = converter(args1_);
            
            pipe(fd); // Create pipe
            
            // Fork child process
            pid_t c = fork();
            if(c<0){
                cout<<"Fork failed\n";
            }
            else if(c==0){ // Child process
                    // Connect input from previous pipe (if any)
                    if(next_fd != 0){
                        dup2(next_fd,0); 
                        close(next_fd);
                    }
                   
                    // Connect output to current pipe write-end
                    dup2(fd[1],1);
                    close(fd[1]);
                    close(fd[0]);
                    
                    // Execute command
                    if(path1_ == "exit"){
                        History_tracker.clear();
                        exit(0);
                    }
                    else if(builtin_execute(path1_)) exit(0);
                    
                    execvp(args_path1_[0],args_path1_.data());
                    cout << cmd1 << ": command not found\n";
                    exit(1);
            }
            // Parent process: Close write end, keep read end for next iteration
            if(next_fd!=0) close(next_fd);
            next_fd = fd[0];
            close(fd[1]); 
        }
        
        // Execute the Final Command in the pipe chain
        if(idx_ != string::npos   && idx_>=1 && (idx_+2)<cmd1.length()){
            children_count++;
            string path2_ = cmd1.substr(idx_+1);
            vector<string> args2_=quotes_splitter(path2_);
            vector<char*> args_path2_ = converter(args2_);
            
            pid_t c1 = fork();
            
            if(c1<0){
                cout<<"Fork failed\n";
            }
            else if(c1==0){ // Child process
                // Connect input from previous pipe
                if(next_fd != 0){
                   dup2(next_fd,0);
                   close(next_fd);
                }
                
                if(path2_=="exit"){
                  History_tracker.clear();
                  exit(0);
                }
                else if(builtin_execute(path2_)) exit(0);
                
                execvp(args_path2_[0],args_path2_.data());
                cout << cmd1 << ": command not found\n";
                exit(1);
            }
        }
        
        if(next_fd != 0) close(next_fd);
        
        // Wait for all piped children to finish
        for(int i=0;i<children_count;i++){
            wait(NULL); 
        }
      }

    // --- EXIT BUILTIN (Main context) ---
    else if(word=="exit"){
      // Auto-save history on exit if HISTFILE is set
      if(path_ != nullptr){
        if(fs::exists(path_)){
          builtin_execute("history -w "+string(path_));
        }
      }
      History_tracker.clear();
      break;
    }
    
    // --- EXECUTE REGULAR COMMANDS (No pipes) ---
    // If NOT a builtin, fork and exec
    else if(!builtin_execute(cmd1)){
      vector<char*> exec_argument = converter(argument);

      pid_t c=fork();

      if(c<0){
          cout<<"Fork failed! (system failed)\n";
      }
      else if(c==0){   
          execvp(argument[0].c_str(),exec_argument.data());  
          cout << cmd1 << ": command not found\n";
          exit(1);
      }
      else{
          wait(NULL);
      }
    }  

    // Restore stdout/stderr if redirection was active
    if(redirection_active){
      dup2(saved_stdout,temp_fd);
      close(saved_stdout);
    }    
  }

  return 0;
}