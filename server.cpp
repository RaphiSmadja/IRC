//g++ -g -Wall -std=c++0x -pthread server.cpp -o server

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>      
#include <sstream> 		//stringstream
#include <string>       // objet de class String
#include <thread>
#include <vector>       // pour faire des tableaux dynamiques conteneur sequence
#include <map> 			//element map conteneurs associatif

#define PORT 666

using namespace std; //On utilise un espace de noms ici

int numberOfRoom=5;
map <int, vector <string>> roomList;
int addrlen;
int playeur=0;
int threadCount=0;
struct sockaddr_in address;
map <string, string> pseudoList;
int server_fd;

//Save a string to a txt file
void saveRoomText(int roomNum){
	string name;
    stringstream ssName;
	FILE * pFile;
	ssName << roomNum << ".txt";
	name = ssName.str();
	pFile = fopen (name.c_str(),"w");
	if (pFile!=NULL){
		for (unsigned int i = 0; i < roomList[roomNum].size(); ++i){
			stringstream ssString;
			ssString << roomList[roomNum][i] << "\n";
			fputs((ssString.str()).c_str(),pFile);			
		}
		fclose (pFile);
	}
}

//Extract the text from a txt file
void readRoomText(int roomNum){
	string name;
	stringstream ssName;
    /** Objects of this class use a string buffer that contains a sequence of characters. 
    This sequence of characters can be accessed directly as a string object, using member str. **/
	FILE * pFile;
	ssName << roomNum << ".txt";
	name = ssName.str();
	pFile = fopen (name.c_str(),"r");  //. c_str() premet de récupérer le char* contenue dans un objet de type string
	if (pFile!=NULL){
		char c;
		do{
			vector<string> requestSplit;
			string line="";
			do{
				c = fgetc(pFile);
				if(c != EOF && c != '\n'){
					stringstream ss;
					ss << line << c;
					line = ss.str();
				}
			}while(c != EOF && c != '\n');
			roomList[roomNum].push_back(line);
		} while(c != EOF);
		fclose (pFile);
	}
}
//Request recovery and transformation into string
string readToString(unsigned  int sock){
    char buffer[1024] = {0};
    signed int valRead;
	string tot;
	while(true){
		valRead = read( sock , buffer,1024);
		if(valRead!=1024){
			buffer[valRead]='\0';
		}
		stringstream ss;
		ss << tot << buffer;
		tot = ss.str();
		if(valRead!=1024){
			return tot;
		}
	}
}
//Divide a request into different string
vector<string> splitRequest(string req){
	vector<string> tab;
	char *str;
	str = const_cast<char *>(req.c_str());
	char * pch;
	pch = strtok (str,"&");
	while (pch != NULL){
		string tmp = pch;
		tab.push_back(tmp);
		pch = strtok (NULL, "&");
	}
	return tab;
}
//Management of different requests of a user
void newSocket(int new_socket){
	int idP=playeur++;
    string comPseu="70";    
    string comInfo="71";
    string comMess="50";
    string comSend="51";
    string msghello = "";
	while(true){
	    string msgRead = readToString(new_socket);
		vector<string> split = splitRequest(msgRead);
	    if(msgRead.length()==0){
	    	return;
	    }
		if(split[0].compare(comPseu)==0){
			string msgReturn ="1";
			msgRead = msgRead.substr(3,msgRead.length()-4);
			pseudoList[msgRead]=msgRead;
			send(new_socket , msgReturn.c_str() , msgReturn.length() , 0 );
			printf("new user %s%d\n",(msgRead.c_str()),idP );
		}
		if(split[0].compare(comInfo)==0){
			string msgInfo ="0&";
			stringstream ss;
			ss << msgInfo << numberOfRoom << "&";
			msgInfo = ss.str();
			send(new_socket , msgInfo.c_str() , msgInfo.length() , 0 );
		}
		if(split[0].compare(comMess)==0){
			stringstream ss;
			for (unsigned int i = 0; i != roomList[atoi(split[1].c_str())].size(); i++){
				ss << roomList[atoi(split[1].c_str())][i] << "\n";
			}
			string msgInfo=ss.str();
			send(new_socket , msgInfo.c_str() , msgInfo.length() , 0 );
		}
		if(split[0].compare(comSend)==0){
			stringstream ss;
			ss << split[2] << "#" << idP << " : " << split[3];
			roomList[atoi(split[1].c_str())].push_back(ss.str());
		}
	}
}
//Backup of different servers on txt files
void saveText(){
	int prevtime=0;
	while(true){
    	time_t result = time(nullptr);
		stringstream ss;
		ss << result;
		if(prevtime < atoi(ss.str().c_str())){
			for (int i = 0; i < numberOfRoom; ++i){
				saveRoomText(i);
			}
    		time_t result2 = time(nullptr);
			stringstream ss2;
			ss2 << result2;
			prevtime=atoi(ss2.str().c_str())+30;
		}
	}
}
//Thread management
void addTread(){
	vector<thread> threads;
	threads.emplace_back(saveText);
	while(true){
		while (playeur+5>=threadCount){
			int clientSocket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen);
			threads.emplace_back(newSocket,clientSocket);
			threadCount++;
			for (thread & t : threads) {
				if(t.joinable()==false)
					t.join();
			}
		}
	}
}
//server setting
void paramServ(){
	string text="";
    int opt = 1;
    addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // Forcefully attaching socket to the port 
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
	addTread();
}
int main(int argc, char const *argv[]){
	for (int i = 0; i < numberOfRoom; ++i){
		readRoomText(i);
		if(roomList[i].size()==0){
            stringstream ss;
			ss << "Server : Hello room " << i << ".";
			roomList[i].push_back(ss.str());
		}
    }
	paramServ();
    return 0;
}