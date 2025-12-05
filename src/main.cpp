#include <bits/stdc++.h>
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
      if(word!="echo" && word!="exit" && word!="type"){
        cout<<word;
        while(ss>>word){
          cout<<" "<<word;
        }
        cout<<": not found\n";
      } 
      else{
        cout<<word<<" is a shell builtin";      
        cout<<"\n";
      }
    }


    else cout<<cmd1<<": command not found\n";
   
  }



}
