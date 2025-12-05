#include <bits/stdc++.h>
#include <sys/stat.h>
using namespace std;





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
      while(ss>>word){
        if(word=="echo") continue;
        cout<<word<<" ";
      }
      cout<<"\n";
      continue;
    }
    
    else if(word=="type"){
      ss>>word;
      if(word!="echo" && word!="exit"){
        string path;
        path=getenv("PATH");
        path=':'+path;
    
        size_t pos = path.find(":");  // position of "live" in str
        size_t second_idx=pos;
        bool found=false;
        while(pos != -1){
            second_idx=path.find(":",second_idx+1);
            string required_path=path.substr(pos+1,second_idx-pos-1);
            pos=second_idx;
    
            //building final path and searching in it
            required_path=required_path+'/'+ path.substr(5);
    
            // if got
            struct stat info;
            int result = stat(required_path.c_str(), &info);
            if(!result){
              //
              if(info.st_mode & _S_IEXEC){
                cout<<cmd1.substr(5)<<" is "<<required_path<<"\n";
                break;
              }
            }          
        }
        if(!found) cout<<cmd1.substr(5)<<": not found\n";
      } 
      else{
        cout<<word<<" is a shell builtin";      
        cout<<"\n";
      }
    }
    else cout<<cmd1<<": command not found\n";
    
  }
  
  

}
