#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace std; 

int main() {
    
    /*string test = "a b c d e f g h";
    stringstream t(test);
    vector<string> out{istream_iterator<string>{t}, istream_iterator<string>{}};
    
    for(unsigned i = 0; i < out.size(); ++i)
    {
        cout << out[i] << endl;
    }*/
    //copy(istream_iterator<string>(test), istream_iterator<string>(), back_inserter(out));
    //istream_iterator<string> begin(t);
    //istream_iterator<string> end;
    //vector<string> out(begin, end);
    //copy(out.begin,out.end);
    
    cout << "Welcome, these are the possible viewable maps:" << endl;
    cout << "1. Cairo, Egypt" <<endl;
    cout << "2. Hamilton, Canda" <<endl;
    cout << "3. Moscow, Russia" <<endl;
    cout << "4. Newyork, USA" <<endl;
    cout << "5. Saint Helena" <<endl;
    cout << "6. Toronto, Canada" <<endl;
    cout << "7. Toronto, Canada (Large)" <<endl;
    cout << "Choose one: " ;
    
    int choice = 0;
    string map;
    
    cin >> choice;
    
    if(choice == 1)
        map = "/cad2/ece297s/public/maps/cairo_egypt.bin";
    else if(choice == 2)
        map = "/cad2/ece297s/public/maps/hamilton_canada.bin";
    else if(choice == 3)
        map = "/cad2/ece297s/public/maps/moscow.bin";
    else if(choice == 4)
        map = "/cad2/ece297s/public/maps/newyork.bin";
    else if(choice == 5)
        map = "/cad2/ece297s/public/maps/saint_helena.bin";
    else if(choice == 6)
        map = "/cad2/ece297s/public/maps/toronto.bin";
    else if(choice == 7)
        map = "/cad2/ece297s/public/maps/toronto_canada.bin";
    
    

    load_map(map);
    draw_map(map);
    close_map();
    
    
    
    

    return 0;
}




























