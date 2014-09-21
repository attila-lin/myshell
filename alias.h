#ifndef ALIAS_H
#define ALIAS_H
#include <map>
#include <iostream>
using namespace std;
class AliasMap
{
public:
  map<string, string> m;
  AliasMap(){
  }
  void addword(string a, string b, int add){
    map<string ,string >::iterator it;;
    it = m.find(a);
    if(it == m.end())
    {
    	if(add){
    		b.insert(0, 1, '\'');
    		b.insert(b.end(), 1,'\'');
    		m.insert(pair<string,string>(a,b));
    	}
      else
      	m.insert(pair<string,string>(a,b));
    }
    else{
    	if (add)
    	{
    		b.insert(0, 1, '\'');
    		b.insert(b.end(), 1,'\'');
    		m[a] = b;
    	}
    	else
      	m[a] = b;
    }
  }
  string search(string a){
    // cout << m[m[a]] << endl;
    string ret = "";
    vector<string> v ;
    map<string ,string >::iterator it;

    it = m.find(a);
    if(it == m.end())
      return "";
    else{
      do{
        ret = it->second;
        ret = ret.substr(1,ret.length()-2);
        v.push_back(it->first);
        int b = 0;
        for (unsigned int i = 0; i < v.size(); ++i)
        {
          if(ret == v[i]){
            ret = v[0];
            b = 1;
            break;
          }
        }
        if(b == 1)
          break;
        it = m.find(ret);
      }while(it != m.end());
    }

    return ret;
  }
  string searchfirst(string a){
    string ret = "";
    map<string ,string >::iterator it;;
    it = m.find(a);
    if(it == m.end())
      return ret;
    else{
      ret = it->second;
    }
    return ret;
  }
  void delword(string a){
    m.erase(a);
  }
  void show(){
    map<string,string>::iterator it=m.begin();
    string first, second;
    for(; it!=m.end(); ++it){
      first = it->first;
      second = it->second;
      cout << "alias " << first << "=" << second << endl;
    }
  }
  ~AliasMap(){}
};

#endif /*ALIAS_H*/