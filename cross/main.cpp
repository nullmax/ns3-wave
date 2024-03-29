#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;
//   <vehicle depart="54000" id="veh0" route="route01" type="CarA" color="1,0,0" /> 

int v_type = 0;

class vehicle
{
private:
    /* data */
    int id;
    int route_id;
    int type_id;
    static int counter;

    char type2char();

public:
    vehicle();
    ~vehicle();
    void disp(char * str);
};

int vehicle::counter = 0;

vehicle::vehicle()
{
    // type_id = rand() % 6;
    type_id = v_type;
}

vehicle::~vehicle()
{
}

char vehicle::type2char()
{
    switch (type_id)
    {
    case 0:
        return 'A';
        break;
    case 1:
        return 'B';
        break;
    case 2:
        return 'C';
        break;
    case 3:
        return 'D';
        break;
    case 4:
        return 'E';
        break;
    case 5:
        return 'F';
        break;
    default:
        return 'A';
        break;
    }
}

void vehicle::disp(char * str)
{
    id = counter++;
    route_id = rand() % 8 + 1;
    sprintf(str, "  <vehicle depart=\"%d\" id=\"veh%d\" route=\"route%d\" type=\"Car%c\" color=\"1,0,0\" />\n", id / 20, id, route_id, type2char());
}

static const uint32_t n_vehicles = 200;

int main(int argc, char** argv) {
    srand(time(0));

    v_type  = argv[1][0] - '0';

    string head = R"(<?xml version="1.0" encoding="UTF-8"?>
<routes>
  <vType id="CarA" maxSpeed="5.0" sigma="0.5" />
  <vType id="CarB" maxSpeed="10.0" sigma="0.5" />
  <vType id="CarC" maxSpeed="15.0" sigma="0.5" />
  <vType id="CarD" maxSpeed="20.0" sigma="0.5" />
  <vType id="CarE" maxSpeed="25.0" sigma="0.5" />
  <vType id="CarF" maxSpeed="30.0" sigma="0.5" />

  <route id="route1" edges="e4 e6"/>
  <route id="route2" edges="e4 e7"/>
  <route id="route3" edges="e1 e3"/>
  <route id="route4" edges="e1 e7"/>
  <route id="route5" edges="e8 e6"/>
  <route id="route6" edges="e8 e2"/>
  <route id="route7" edges="e5 e3"/>
  <route id="route8" edges="e5 e2"/>)";

    cout << head << endl;
    cout << endl;
    char str[80];
    vehicle vs;
    for (int i=0; i < n_vehicles; ++i)
    {
        vs.disp(str);
        cout<<str;
    }
    string tail = R"(</routes>)";
    cout << tail << endl;
}
