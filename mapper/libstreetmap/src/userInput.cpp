#include "userInput.h"

using namespace std;

userInput::userInput()
{
    users_input="";
    enabled=false;
    first_search_inter=false;
    first_search_poi=false;
}

userInput::userInput(string users)
{
    users_input=users;
    enabled=false;
    first_search_inter=false;
    first_search_poi=false;
}

userInput::~userInput()
{
    
}

void userInput::setEnabled(bool input)
{
    enabled=input;
}

bool userInput::getEnable()
{
    return enabled;
}

void userInput::set_users_input(string input)
{
    users_input = input;
    cout << input << endl;
    if(input!="")
    {
        enabled=true;
        first_search_inter=false;
        first_search_inter=true;
    } else {
        enabled=false;
    }
    find_match_intersections(input);
    find_match_poi(input);
   // sort_matching_intersections(input);
   // sort_matching_poi(input);
}

string userInput::get_users_input()
{
    return users_input;
}

vector<vector<inter_information>> userInput::get_intersections()
{
    return intersections;
}

vector<vector<inter_information>> userInput::get_poi()
{
    return poi;
}

void userInput::organize_intersections()
{
    unsigned unknownc = 0;
    
    inter_information datas;
    unsigned long long numberOfIntersections = getNumberOfIntersections();
   // cout << numberOfIntersections << endl;
    intersections.resize(28);
    for(int i=0; i< (int)numberOfIntersections;i++)
    {
        string immName = getIntersectionName(i);
        string insertName = ConvertToCaps(getIntersectionName(i));
        datas.id=i;
        if((getIntersectionName(i).find("unknown") != string::npos) && unknownc < 3)
        {
            if(unknownc == 0) insertName = "Christopher Enriquez";
            else if(unknownc == 1) insertName = "Taihyun Lee";
            else insertName = "Sampson Gao";   
            unknownc ++;
        }
        datas.inter_name=insertName;
       // if(insertName.find("UNKNOWN")==std::string::npos)
       // {
            //cout << i << endl;
           // cout << getIntersectionName(i) << endl;
           // cout << insertName << endl;
            int firstCharacter = getfirstcharacter(insertName)-65;
          //  cout << firstCharacter << endl;
            if(firstCharacter>=0&&firstCharacter<=25)
            {
        //cout << firstCharacter << endl;
                intersections[firstCharacter].push_back(datas);
            } else {
                continue;
            }
       // }
        
        string convertedName = switchWords(immName);
        string convertCapName = ConvertToCaps(convertedName);
        datas.id=i;
        datas.inter_name=convertCapName;
        firstCharacter = getfirstcharacter(convertCapName)-65;
        if(firstCharacter>=0&&firstCharacter<=25)
        {
            intersections[firstCharacter].push_back(datas);
        } else {
            continue;
        }
    }
    //cout << intersections[18].at(1) << endl;
}

void userInput::organize_poi()
{
    inter_information datas;
    unsigned long long numberOfPoi = getNumberOfPointsOfInterest();
    poi.resize(28);
    //cout << numberOfPoi << endl;
    for(int i=0; i< (int)numberOfPoi;i++)
    {
        string insertPoiName = ConvertToCaps(getPointOfInterestName(i));
        datas.id=i;
        datas.inter_name=insertPoiName;
        int firstCharacter = getfirstcharacter(insertPoiName)-65;
        if(firstCharacter>=0 && firstCharacter<=25)
        {
            poi[firstCharacter].push_back(datas);
           // cout << poi[firstCharacter].at(i) << endl;
        } else 
        {
            continue;
        }
    }
}

string userInput::ConvertToCaps(string input)
{
    string returningString="";
    for(int i=0;i<(int)input.size();i++)
    {
        if(input[i]!='\0' && input[i]>=97 && input[i]<=122 && input[i]!='&')
        {
            char lowercase= input[i]-32;
            returningString+=lowercase;
        } else {
            returningString+=input[i];
        }
    }
    return returningString;
}

string userInput::switchWords(string input)
{
    string returningString="";
    string firstword="", secondword=input;
   // size_t pos;
    bool checkAmpersand = false;
    for(size_t i=0;i<input.size();i++)
    {
        if(input[i]=='&')
        {
            checkAmpersand = true;
            break;
        } else{
            //secondword = input;
          //  pos = i;
            firstword+=input[i];
            secondword.erase(secondword.begin());
        }
    }
    if(checkAmpersand == false)
    {
        return returningString;
    }
    secondword.erase(secondword.begin()); //gets rid of &
    secondword.erase(secondword.begin()); //gets rid of the space after &
    returningString = secondword + " & " + firstword;
    return returningString;
}

char userInput::getfirstcharacter(string input)
{
    char returnInt;
    for(size_t i=0; i<input.size();i++)
    {
        if(input[i]>=65 && input[i]<=90 && input[i]!='&')
        {
            returnInt=input[i];
            break;
        }
    }
    if(returnInt==-1)
    {
        return NULL;
    }
    return returnInt;
}

void userInput::find_match_intersections(string input)
{
    inter_information datas;
    if(!matching_intersections.empty())
    {
        matching_intersections.clear();
    }
   // if(!first_search_inter)
   // {
        string convertedUserInput = ConvertToCaps(input);
       // cout << input << endl;
        //cout << convertedUserInput << endl;
        int firstcharacter = getfirstcharacter(convertedUserInput)-65;
        datas.id=-1;
        datas.inter_name="";
        matching_intersections.push_back(datas);
        //cout << firstcharacter << endl;
        for(int i=0; i<(int)intersections[firstcharacter].size();i++)
        {
            //cout << "wth" << endl;
            size_t found = intersections[firstcharacter].at(i).inter_name.find(convertedUserInput);
            if(found!=string::npos)
            {
               // cout << "wth2" << endl;
                matching_intersections.push_back(intersections[firstcharacter].at(i));
                //cout << intersections[firstcharacter].at(i) << endl;
            }
        }
   //     first_search_inter = true;
   // }
    /*string switchedInput = switchWords(input);
    cout << switchedInput << endl;
    string capinput = ConvertToCaps(switchedInput);
    cout << capinput << endl;
    firstcharacter = getfirstcharacter(capinput)-65;
    cout << firstcharacter << endl;
    for(int i=0; i<intersections[firstcharacter].size();i++)
    {
        if(capinput == intersections[firstcharacter].at(i))
        {
            matching_intersections.push_back(intersections[firstcharacter].at(i));
        }
    }*/
    //cout << matching_intersections.at(0) << endl;
}

void userInput::find_match_poi(string input)
{
    inter_information datas;
    if(!matching_poi.empty())
    {
        matching_poi.clear();
    }
    //matching_poi.push_back()
    //if(!first_search_poi)
    //{
        string convertedUserInput = ConvertToCaps(input);
        //cout << input << endl;
       // cout << convertedUserInput << endl;
        int firstcharacter = getfirstcharacter(convertedUserInput)-65;
        datas.id=-1;
        datas.inter_name="";
        matching_poi.push_back(datas);
        //cout << firstcharacter << endl;
        for(int i=0; i<(int)poi[firstcharacter].size();i++)
        {
            size_t found = poi[firstcharacter].at(i).inter_name.find(convertedUserInput);
            if(found!=string::npos)
            {
                matching_poi.push_back(poi[firstcharacter].at(i));
                //cout << poi[firstcharacter].at(i) << endl;
            }
        }
     //   first_search_poi = true;
    //}
    /*string switchedInput = switchWords(users_input);
    string capinput = ConvertToCaps(switchedInput);
    firstcharacter = getfirstcharacter(capinput)-65;
    for(int i=0; i<poi[firstcharacter].size();i++)
    {
        if(capinput == poi[firstcharacter].at(i))
        {
            matching_poi.push_back(poi[firstcharacter].at(i));
        }
    }*/
}

string userInput::get_first_intersection()
{
    string s="";
    if(matching_intersections.empty())
    {
        return s;
    } else {
        if(matching_intersections.size()!=1)
        {
            return matching_intersections.at(1).inter_name;
        } else 
        {
            return matching_intersections.at(0).inter_name;
        }
    }
}

string userInput::get_first_poi()
{
    string s="";
    if(matching_intersections.empty())
    {
        return s;
    } else {
        if(matching_poi.size()!=1)
        {
            return matching_poi.at(1).inter_name;
        }else
        {
            return matching_poi.at(0).inter_name;
        }
    }
}

unsigned userInput::get_first_intersectionID()
{
    if(matching_intersections.empty())
    {
        return NULL; 
    } else 
    {
        if(matching_intersections.size()!=1)
        {
            return matching_intersections.at(1).id;
        }else
        {
            return matching_intersections.at(0).id;
        }
    }
}

unsigned userInput::get_first_poiID()
{
    if(matching_poi.empty())
    {
        return NULL; 
    } else 
    {
        if(matching_poi.size()!=1)
        {
            return matching_poi.at(1).id;
        }else
        {
            return matching_poi.at(0).id;
        }
    }
}

void userInput::sort_matching_intersections(string input)
{
    if(first_search_inter)
    {
        string convertedUserInput = ConvertToCaps(input);
        for(int i=0;i<(int)matching_intersections.size();i++)
        {
            size_t found = matching_intersections.at(i).inter_name.find(convertedUserInput);
            if(found==string::npos)
            {
                matching_intersections.erase(matching_intersections.begin()+i);
            }
        }
    }
}

void userInput::sort_matching_poi(string input)
{
    if(first_search_inter)
    {
        string convertedUserInput = ConvertToCaps(input);
        for(int i=0;i<(int)matching_poi.size();i++)
        {
            size_t found = matching_poi.at(i).inter_name.find(convertedUserInput);
            if(found==string::npos)
            {
                matching_poi.erase(matching_intersections.begin()+i);
            }
        }
    }
}

void userInput::set_search_first(bool input)
{
    first_search_inter = input;
    first_search_poi = input;
}

void userInput::print(std::string output)
{
    int spacefound = 0;
    users_input=output;
    for(int i=0;i<(int)output.size();i++)
    {
        if(output[i]=='&')
        {
            spacefound=0;
            continue;
        }
        if((i!=((int)output.size()-1))&&output[i+1]>=65 && output[i]==' ')
        {
            continue;
        } else if(output[i]<97&& output[i]>=65 && output[i]!='\0')
        {
            output[i] += 32;
        } else
        {
            spacefound++;
        }
        cout << output[i];
    }
    cout << endl;
    cout << users_input << endl;
}







//newly commited