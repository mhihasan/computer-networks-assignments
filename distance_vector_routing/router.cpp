#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include<thread>
#include<bits/stdc++.h>
using namespace std;

#define MAX 10000
#define MAXCHAR 2048

	int sockfd;
	int bind_flag;
	int sent_bytes;
	int bytes_received;
	char buffer[MAXCHAR];

	socklen_t addrlen;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	struct sockaddr_in dummy;


    string ip1, ip2;
    int cost;




struct routing_table_entry{
	string next_hop;
	int cost;
    routing_table_entry(){
    }


	routing_table_entry(string next_hop, int cost){
        this->next_hop=next_hop;
        this->cost=cost;

	}

};

map<string, routing_table_entry> routing_table;
map <string, routing_table_entry > ::iterator it ,nest, temp;



void print_routing_table(){

it= routing_table.begin();

cout<<"-----------------------------Routing table----------------------------------"<<endl;
cout<<endl;

cout<<"destination"<<"          "<<"next hop"<<"         "<<"cost"<<endl;
cout<<"-----------"<<"          "<<"--------"<<"         "<<"----"<<endl;

while(it!= routing_table.end()){

    if( ((it->second).next_hop).compare("-") )
        cout<< it->first <<"         "<< (it->second).next_hop<<"      "<<(it->second).cost<<endl;
    else
        cout<< it->first <<"          "<< (it->second).next_hop<<"               "<<(it->second).cost<<endl;
    it++;

}
cout<<endl;
cout<<"--------------------------------------------------------------------------"<<endl;
cout<<endl;
}

string myIP;


void send_table_to_neighbor(){

    it = routing_table.begin();

    ostringstream message;

    while(it!= routing_table.end()){

        if((it->first).compare((it->second).next_hop) ==0){ //neighbour check

            cout<<"Sending table to: "<<it->first<<endl;
            inet_pton(AF_INET,(it->first).c_str(),&server_address.sin_addr);
            cout<<"Neighbour IP: "<<it->first<<endl;
            nest = routing_table.begin();

            while(nest != routing_table.end()){
                message.clear();
                message.str("");
                // destination next hop cost
                message<<"route_update"<<" "<< (nest->first)<<" "<<myIP<<" "<<(nest->second).cost<<" "; //destination

                string message_string = message.str();
                strcpy((char*)buffer, message_string.c_str());
                cout<<"Message string: "<<message_string<<endl;

                printf("Sending to: [%s:%hu]: %s\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port), buffer);

                sent_bytes=sendto(sockfd, buffer, MAXCHAR, 0, (struct sockaddr*) &server_address, sizeof(sockaddr_in));
               // int random = rand() % 5 + 2;

             //   std::this_thread::sleep_for (std::chrono::seconds(random));

                nest++;
            }

       // cout<<buffer <<endl;

       }


        it++;


    }

    return;

}

int _count =0;

void update_routing_table(){
    cout<<"My IP: "<<myIP<<endl;

	while(true){
//		gets(buffer);
        cout<<"Am I ever here?"<<endl;
        string data(buffer);

	    //cout<<string(buffer)<<endl;
	    addrlen= sizeof(dummy);

		bytes_received = recvfrom(sockfd, buffer, strlen(buffer) + 1 , 0, (struct sockaddr*) &dummy, &addrlen);

		printf("Received from: [%s:%hu]: %s\n", inet_ntoa(dummy.sin_addr), ntohs(dummy.sin_port), buffer);


        if(! data.compare("shutdown"))
            _count++;
        if(_count==2)
            break;

//        string loopback = string(inet_ntoa(dummy.sin_addr));
//
//        if(loopback.compare("0.0.0.0")){
//
//            cout<<"Loop back detected."<<endl;
//            continue;
//
//        }
		//cout<<"Bytes received: "<<bytes_received<<endl;
    //    printf("[%s:%hu]: %s\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port), buffer);


        //cout<< "Certainly not."<<endl;


        istringstream data_buff(data);


        string dest, next_hop;
        data_buff>>dest>>next_hop>>cost;
         cout<<"Receiving: "<<dest<<" "<<next_hop<<" "<<cost<<endl;

        it= routing_table.find(dest);

        if(it != routing_table.end()){

            //update the table
            temp = routing_table.find(next_hop); //next_hop is my neigbour

            if((temp->second).cost + cost < (it->second).cost ){ // A-B-X , (A->B) + (B->X) < (A->X)
                //it->second = routing_table_entry(next_hop, cost);
                routing_table[it->first]=routing_table_entry(next_hop, (temp->second).cost + cost);
                cout<<"Inside."<<endl;
                cout<<dest<<" "<<next_hop<<" "<<cost<<endl;
                cout<<it->first<<" "<<(it->second).next_hop<<" "<<(it->second).cost<<endl;
                cout<<"DEBUG"<<endl;



              //  print_routing_table();


            }


        }
      //  std::this_thread::sleep_for (std::chrono::seconds(random));


	}

    return;

}


void config(char *argv){


	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(4747);

    client_address.sin_family = AF_INET;
	client_address.sin_port = htons(4747);
	//client_address.sin_addr.s_addr = inet_addr(argv[1]);
	inet_pton(AF_INET,argv,&client_address.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bind_flag = bind(sockfd, (struct sockaddr*) &client_address, sizeof(sockaddr_in));

	if(bind_flag==0)
        printf("successful bind\n");


}


void init_routing_table(char *argv){

ifstream file("topo.txt");

     myIP=string(argv);

    string hold;

    while(getline(file, hold)){

        istringstream input(hold);
        input>>ip1>>ip2>>cost;

        if(!myIP.compare(ip1))

            routing_table[ip2]=routing_table_entry(ip2, cost); // destination ip2, next hop ip2

        else if(!myIP.compare(ip2))

            routing_table[ip1]=routing_table_entry(ip1, cost); //destination ip1, next hop ip1

        else{

                it= routing_table.find(ip1);

                if(it == routing_table.end())
                    routing_table[ip1]= routing_table_entry("-", MAX);

                it= routing_table.find(ip2);


                if(it == routing_table.end())
                    routing_table[ip2]= routing_table_entry("-", MAX);
        }
    }

    file.close();



}



int main(int argc, char *argv[]){


    int cost;
    string command, dest, next_hop;


	if(argc != 2){
		printf("%s <ip address>\n", argv[0]);
		exit(1);
	}


    config(argv[1]);
    init_routing_table(argv[1]);
    print_routing_table();



    while(true){


	    addrlen= sizeof(dummy);
		bytes_received = recvfrom(sockfd, buffer, MAXCHAR , 0, (struct sockaddr*) &dummy, &addrlen);

        printf("Received from: [%s:%hu]: %s\n", inet_ntoa(dummy.sin_addr), ntohs(dummy.sin_port), buffer);

        string test(buffer);

        if(test.find("show") !=  std::string::npos){
            //cout<<int((unsigned char)test[4])<<"."<<int((unsigned char)test[5])<<"."<<int((unsigned char)test[6])<<"."<<int((unsigned char)test[7])<<endl;
            //printf("--------------Showing the routing table.------------\n");
            print_routing_table();


        }

        else if(test.find("clk") != std::string::npos){
             cout<<"TEST"<<endl;
             send_table_to_neighbor();


        }

        else if(test.find("route_update") != std::string::npos){

                cout<<"--------------------------------------------HELLO-------------------------------------------"<<endl;

                istringstream data_buff(test);
                data_buff>>command>>dest>>next_hop>>cost;
                cout<<command<<" "<<dest<<" "<<next_hop<<" "<<cost<<endl;


                it= routing_table.find(dest); //destination


                 if(it != routing_table.end()){


                        cout<<"------------------------HELLO FIRST INSIDE----------------------------------"<<endl;

                    //update the table
                      temp = routing_table.find(next_hop); //my neigbour's IP


                     if(temp != routing_table.end()){

                        cout<<"------------------------HELLO NEXT INSIDE----------------------------------"<<endl;
                        cout<<(temp->second).cost + cost <<"         "<< (it->second).cost<<endl;
                        cout<<"----------------------------------------------------------------------------"<<endl;

          //cost for going to neighbour + neighbour to destination < cost of going from my IP to destination
                         if((temp->second).cost + cost < (it->second).cost ){ // A-B-X , (A->B) + (B->X) < (A->X)
                        //destination's next_hop is neighbour_IP, new cost for destination
                                    routing_table[it->first]=routing_table_entry(next_hop, (temp->second).cost + cost);
                                    cout<<"-----------------------------Inside.--------------------------------------------"<<endl;
                                    cout<<dest<<" "<<next_hop<<" "<<cost<<endl;
                                    cout<<it->first<<" "<<(it->second).next_hop<<" "<<(it->second).cost<<endl;

                                }
                        }

                        else {
                                            cout<<next_hop<<" next_hop is: "<<"-----------------------------BUGG--------------------------------------------"<<endl;


                        }

        }

                else{

                                        cout<<dest<<" destination is: "<<"-----------------------------BUGG--------------------------------------------"<<endl;

                }


}


    }





	close(sockfd);
	return 0;

}
