#include <bits/stdc++.h>
# include <filesystem>
# include <sys/wait.h>

using namespace std;
namespace fs = filesystem;

vector<string> quotes_splitter(string &str){
    vector<string> final;
    bool in_quotes=false;
    string temp="";
    int num=0;
    for(int c=0;c<str.length();c++){
        // cout<<c;
        if(str[c]=='\\'){
            if(c==str.length()-1) temp+=str[c];
            else{
                c++;
                temp+=str[c];
            }
        }
        else if(str[c]=='\'' || str[c]=='\"'){
            if(in_quotes==false){
              in_quotes=true;
              if(str[c]=='\'') num=1;
              else num=2;
            } 
            else{
              if((num==1 && c=='\'') || (num==2 && c=='\"')) in_quotes=false;
              else temp+=str[c]; //if(num==2 && c=='\'') temp+=c; 
            }
        }
        else if(str[c]==' ' && !in_quotes){
            if(temp!=""){
                final.push_back(temp);
                // final" ";
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

//function to splite a string about ':'
vector<string> splitter(string &str,char s){
  vector<string> ans;
  size_t start=0;
  size_t end=str.find(s);
  while(end!=-1){
      ans.push_back(str.substr(start,end-start));
    // // here i am using the string_view instead of substr and reason you can know
    //   ans.push_back(string_view(start,end-start));
      start=end+1;
      end=str.find(s,start);
  }
  ans.push_back(str.substr(start));
//   ans.push_back(string_view(start));
  return ans;
}

string path=getenv("PATH");
vector<string> directry=splitter(path,':');

vector<char*> converter(vector<string>& vec){
    vector<char*> argv;
    for(int i=0;i<vec.size();i++){
        argv.push_back(const_cast<char*>(vec[i].c_str()));
    }
    argv.push_back(nullptr);
    
    return argv;
}

int main() {
  // Flush after every std::cout / std:cerr
  cout << std::unitbuf;
  cerr << std::unitbuf;


  // i had to write the command not found until user doesn't stop
  while(true){
    cout<<"$ ";
    string cmd1;
    getline(cin,cmd1);
    stringstream ss(cmd1);
    string word;
    ss>>word;

    if(cmd1=="exit") break; // implementing the exit builtin
    
    else if(word=="echo"){
      if(cmd1.length()>5){
        string abc = cmd1.substr(5);
        for(auto v: quotes_splitter(abc)){
          cout<<v<<" ";
        }
        // cout<<quotes_splitter(abc);
      }
      cout<<"\n";
    }

    else if(word=="type"){           
      ss>>word;
      if(word=="echo"){
        cout<<"echo is a shell builtin\n";
      }
      else if(word=="exit"){
        cout<<"exit is a shell builtin\n";
      }
      else if(word=="type" || word=="pwd" || word == "cd"){
        cout<<word<<" is a shell builtin\n";
      }
      else{
        string file_name=cmd1.substr(5);
        bool file_done=false;

        for(auto each_path:directry){

          //creating whole path for that file
          auto new_path=each_path+'/'+file_name;

          if(fs::exists(new_path) && fs::is_regular_file(new_path)){
            fs::file_status s = fs::status(new_path);

            fs::perms p = s.permissions();

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
    }

    else if(word == "pwd"){
      cout<<fs::current_path().string()<<"\n";
    }

    else if(word == "cd"){
      ss>>word;
      if(word == "~"){
        /*
        it returns a pointer and why we are not directly storing it in fs::path. As, if it returns nullptr so it will crash the program if we store
        it in fs::path so that is why
        */
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
        string temp_directory=cmd1.substr(3);
        if(fs::exists(temp_directory) && fs::is_directory(temp_directory)){
          fs::current_path(temp_directory);
        }
        else{
          cout<<"cd: "<<temp_directory<<": No such file or directory\n";
        }
      }
    }

    else{
      vector<string> argument=quotes_splitter(cmd1);
      vector<char*> exec_argument = converter(argument);

      pid_t c=fork();

      if(c<0){
          cout<<"Fork failed! (system failed)\n";
      }
      else if(c==0){
        //   vector<char*> exec_argument=converter(argument);
          execvp(argument[0].c_str(),exec_argument.data());  
          
          //this you have to print as a error message
          cout << cmd1 << ": command not found\n";
          exit(1);
      }
      else{
          wait(NULL);
      }


    }
   
  }



}