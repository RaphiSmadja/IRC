//g++ -g -Wall -std=c++0x -pthread server.cpp -o server

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <sstream> 		//stringstream
#include <string>
#include <thread>
#include <vector>
#include <map> 			//element map

#define PORT 8001

int maxRoom=5;
std::map <int, std::vector <std::string>> roomList;
int addrlen;
int playeur=0;
int threadCount=0;
struct sockaddr_in address;
std::map <std::string, std::string> pseudoList;
int server_fd;

//Save a string to a txt file
void saveRoomText(int roomNum){
	std::string name;
	std::stringstream ssName;
	FILE * pFile;
	ssName << roomNum << ".txt";
	name = ssName.str();
	pFile = fopen (name.c_str(),"w");
	if (pFile!=NULL){
		for (unsigned int i = 0; i < roomList[roomNum].size(); ++i){
			std::stringstream ssString;
			ssString << roomList[roomNum][i] << "\n";
			fputs((ssString.str()).c_str(),pFile);			
		}
		fclose (pFile);
	}
}
//Extract the text from a txt file
void readRoomText(int roomNum){
	std::string name;
	std::stringstream ssName;
	FILE * pFile;
	ssName << roomNum << ".txt";
	name = ssName.str();
	pFile = fopen (name.c_str(),"r");
	if (pFile!=NULL){
		char c;
		do{
			std::vector<std::string> requestSplit;
			std::string line="";
			do{
				c = fgetc(pFile);
				if(c != EOF && c != '\n'){
					std::stringstream ss;
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
std::string readToString(unsigned  int sock){
    char buffer[1024] = {0};
    signed int valRead;
	std::string tot;
	while(true){
		valRead = read( sock , buffer,1024);
		if(valRead!=1024){
			buffer[valRead]='\0';
		}
		std::stringstream ss;
		ss << tot << buffer;
		tot = ss.str();
		if(valRead!=1024){
			return tot;
		}
	}
}
//Divide a request into different string
std::vector<std::string> splitRequest(std::string req){
	std::vector<std::string> tab;
	char *str;
	str = const_cast<char *>(req.c_str());
	char * pch;
	pch = strtok (str,"&");
	while (pch != NULL){
		std::string tmp = pch;
		tab.push_back(tmp);
		pch = strtok (NULL, "&");
	}
	return tab;
}
//Management of different requests of a user
void newSocket(int new_socket){
	int idP=playeur++;
    std::string comPseu="70";    
    std::string comInfo="71";
    std::string comMess="50";
    std::string comSend="51";
    std::string msghello = "";
	while(true){
	    std::string msgRead = readToString(new_socket);
		std::vector<std::string> split = splitRequest(msgRead);
	    if(msgRead.length()==0){
	    	return;
	    }
		if(split[0].compare(comPseu)==0){
			std::string msgReturn ="1";
			msgRead = msgRead.substr(3,msgRead.length()-4);
			pseudoList[msgRead]=msgRead;
			send(new_socket , msgReturn.c_str() , msgReturn.length() , 0 );
			printf("new user %s%d\n",(msgRead.c_str()),idP );
		}
		if(split[0].compare(comInfo)==0){
			std::string msgInfo ="0&";
			std::stringstream ss;
			ss << msgInfo << maxRoom << "&";
			msgInfo = ss.str();
			send(new_socket , msgInfo.c_str() , msgInfo.length() , 0 );
		}
		if(split[0].compare(comMess)==0){
			std::stringstream ss;
			for (unsigned int i = 0; i != roomList[atoi(split[1].c_str())].size(); i++){
				ss << roomList[atoi(split[1].c_str())][i] << "\n";
			}
			std::string msgInfo=ss.str();
			send(new_socket , msgInfo.c_str() , msgInfo.length() , 0 );
		}
		if(split[0].compare(comSend)==0){
			std::stringstream ss;
			ss << split[2] << "#" << idP << " : " << split[3];
			roomList[atoi(split[1].c_str())].push_back(ss.str());
		}
	}
}
//Backup of different servers on txt files
void saveText(){
	int prevtime=0;
	while(true){
    	std::time_t result = std::time(nullptr);
		std::stringstream ss;
		ss << result;
		if(prevtime < atoi(ss.str().c_str())){
			for (int i = 0; i < maxRoom; ++i){
				saveRoomText(i);
			}
    		std::time_t result2 = std::time(nullptr);
			std::stringstream ss2;
			ss2 << result2;
			prevtime=atoi(ss2.str().c_str())+30;
		}
	}
}
//Thread management
void addTread(){
	std::vector<std::thread> threads;
	threads.emplace_back(saveText);
	while(true){
		while (playeur+5>=threadCount){
			int clientSocket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen);
			threads.emplace_back(newSocket,clientSocket);
			threadCount++;
			for (std::thread & t : threads) {
				if(t.joinable()==false)
					t.join();
			}
		}
	}
}
//server setting
void paramServ(){
	std::string text="";
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
	for (int i = 0; i < maxRoom; ++i){
		readRoomText(i);
		if(roomList[i].size()==0){
			std::stringstream ss;
			ss << "Server : Hello room " << i << ".";
			roomList[i].push_back(ss.str());
		}
    }
	paramServ();
    return 0;
}