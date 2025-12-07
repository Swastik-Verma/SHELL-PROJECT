#include <bits/stdc++.h>
# include <filesystem>
using namespace std;
namespace fs = filesystem;

vector<string> splitter(string str){
  vector<string> ans;
  size_t start=0;
  size_t end=str.find(':');
  while(end!=-1){
      ans.push_back(str.substr(start,end-start));
      start=end+1;
      end=str.find(':',start);
  }
  ans.push_back(str.substr(start));
  return ans;
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
      cout<<cmd1.substr(5);
      cout<<"\n";
      continue;
    }

    else if(word=="type"){
      ss>>word;
      if(word=="echo"){
        cout<<"echo is a shell builtin\n";
      }
      else if(word=="exit"){
        cout<<"exit is a shell builtin\n";
      }
      else if(word=="type"){
        cout<<"type is a shell builtin\n";
      }
      else{
        string temp=cmd1.substr(5);
        string path=getenv("PATH");
        vector<string> directry=splitter(path);
        bool file_done=false;

        for(auto each_path:directry){
          auto new_path=each_path+'/'+cmd1.substr(5);
          if(fs::exists(new_path) && fs::is_regular_file(new_path)){
            fs::file_status s = fs::status(new_path);

            fs::perms p = s.permissions();

            bool isExecutable = (p & fs::perms::owner_exec) != fs::perms::none;

            if(isExecutable){
              cout<<cmd1.substr(5)<<" is "<<new_path<<"\n";
              file_done=true;
              break;
            }
          }
        }
        if(file_done==false){         
          cout<<cmd1.substr(5)<<": not found\n";            
        }



        // if(1){
        //   //here if found print path
        // }
        // else{
        //   cout<<cmd1.substr(5)<<": not found\n";
        // }
      }
    }

    else cout<<cmd1<<": command not found\n";
   
  }



}