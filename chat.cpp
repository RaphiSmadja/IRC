//g++ -g -Wall chat.cpp -o chat

#include <arpa/inet.h>	//sockaddr_in et htons
#include <termios.h> 	//oldt, termios, ICANON, TCSANOW, tcgetattr, ECHO
#include <unistd.h> 	//read et STDIN_FILENO
#include <stdlib.h>		//system et atoi
#include <iostream>		//cout et cin
#include <stdio.h> 		//getchar et printf
#include <cstring>		//memset
#include <sstream> 		//stringstream
#include <cerrno>		//errno et EINTR
#include <vector> 		//element vector
#include <map> 			//element map
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <ctime>

using namespace std; //On utilise un espace de noms ici

//kbhit version linux s'active lors d'une pression sur le clavier
int kbhit (void){
  struct timeval tv;
  fd_set rdfs;
 
  tv.tv_sec = 0;
  tv.tv_usec = 0;
 
  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);
 
  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);
}
//Verification and socket creation
bool checkSocket(unsigned int *sock){
    int errorCount=0;
  	while(true){
		if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("Erreur de création de Socket.\n");
			errorCount++;
			if(errorCount > 100){
				printf("Trop d'essai.\n");
				return false;
			}
		}else{
			return true;
		}
	}
}
//Management and initialization of the connection
bool checkServer(string *ip,struct sockaddr_in *serv_addr,unsigned int *port,unsigned int sock,bool ipvalid){
	(*serv_addr).sin_family = AF_INET;
	(*serv_addr).sin_port = htons(*port);
	//server connection
	if(inet_pton(AF_INET, (*ip).c_str(), &(*serv_addr).sin_addr)<=0)
	{
		printf("Adresse invalide(%s:%d)/ Adresse non prise en charge \n",(*ip).c_str() ,*port);
	}else{
		if (connect(sock, (struct sockaddr *)&*serv_addr, sizeof(*serv_addr)) < 0)
		{	
			return false;
			printf("La connexion a échoué.(client not synchro with serv)\n");
		}else{
			return true;
		}
	}
}
//sending the nickname to the server and save nickname
bool checkPseudo(string *pseudo,unsigned  int sock){
    signed int valRead;
    unsigned int minSizepseudo=3;
    string sockPseudo="pseudoSend";  
    char buffer[1024] = {0};
  	while(true){
		string tmp;
	  	while((*pseudo).length()<minSizepseudo){
			printf("Entrer votre pseudo:\n");
			cin >> (*pseudo);
	  		if((*pseudo).length()<minSizepseudo){
				printf("Votre pseudo est trop court (%d caractère minimum).\n",minSizepseudo);
	  		}
			stringstream ss;
			ss << sockPseudo << "&" << (*pseudo) << "&";
			string tmp = ss.str();
			if((*pseudo).length()>minSizepseudo){
    			send(sock , tmp.c_str() , tmp.length() , 0 );
			}
	  	}
    	valRead = read(sock ,buffer,1337);
    	if(valRead != 1337){
    		if(buffer[0]!='1'){
    			tmp="";
    		}else{
    			return true;
    		}
    	}
    }
}
//Request recovery and transformation into string
string readToString(unsigned  int sock){
    char buffer[1024] = {0};
    int valRead;
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
void clearIn(){while ( getchar() != '\n' );}
//room management
int roomGest(string menuInfo,map <int, int> *roomList){
	vector<string> tab;
	char *str;
	str = const_cast<char *>(menuInfo.c_str());
	char * pch;
	pch = strtok (str,"&");
	while (pch != NULL){
		string tmp = pch;
		tab.push_back(tmp);
		pch = strtok (NULL, "&");
	}
	for (int i = 0; i < atoi(tab[1].c_str()); i++){
		(*roomList)[i]=i;
	}
	return atoi(tab[1].c_str());
}

int main(int argc, char const *argv[]){
    struct sockaddr_in serv_addr;	// sockaddr
	map <int, int> roomList;
	string previousHist="";
	string menuInfo="";
    string pseudo="";
    string msg="";
	string Hist="";
    string comInfo="infoSend";
    string comMess="messageSend";
    string comSend="sockSend";
    string ip="127.0.0.1";
    int choice = 0;
    unsigned int port = 666;
    unsigned int chanel = 0;
    unsigned int sock = 0;
    bool checkMenuQuit = false;
    bool checkQuit = false;
    bool checksend = false;
	int roomNumber=0;
    if(!checkSocket(&sock)){
    	return -1;
    }
    if(!checkServer(&ip,&serv_addr,&port,sock,true)){	
    	return -1;
    }
	if(!checkPseudo(&pseudo,sock)){
    	return -1;
    }
	while(true){
		checkQuit = false;
    	checkMenuQuit = false;
		send(sock , comInfo.c_str() , comInfo.length() , 0 );
		menuInfo = readToString(sock);
		roomNumber = roomGest(menuInfo,&roomList);
		msg="";
		while(!checkMenuQuit){
			system("clear");
			printf("\t\t\tServeur n°%s\n",ip.c_str());
			printf("0 . Quitter le programme\n");
			for (int i = 0; i < roomNumber; ++i){
				printf("%d . Salle %d\t",i+1,roomList[i]);
			}
			printf("\nEntrez votre choix:\n");
			cin >> msg;
			if(msg.compare("0")==0){
				return -1;
			}
			int intMsg = atoi(msg.c_str())-1;
			if(0 <= intMsg && intMsg <= roomNumber){
				chanel=roomList[intMsg];
				checkMenuQuit=true;
				break;
			}
			msg="";
			clearIn();
		}
		checksend=true;
		while(!checkQuit){
			while(!kbhit()){
				if (Hist.compare(previousHist)!=0||checksend==true){
					system("clear");
					cout << Hist;
					previousHist = Hist;
					printf("\tAppuyez sur échap pour quitter et sur Enter pour écrire.\n");
					checksend=false;
				}
				stringstream ss;
				ss << comMess << "&" << chanel << "&";
			    send(sock , (ss.str()).c_str() , (ss.str()).length() , 0 );
			    Hist = readToString(sock);
			}
			int ch;
			ch=getchar();
			if(10==ch){//enter
				checksend=true;
				printf("Entrer votre text:\n");
				cin >> msg;
				stringstream ss;
				ss << comSend << "&" << chanel << "&" <<pseudo << "&" << msg.c_str() << "&";
				msg = ss.str();
				send(sock , msg.c_str() , msg.length() , 0 );
			}else if(27==ch){//esc
				printf("Appuyez sur Entrée.\n");
				checkQuit=true;
			}
			clearIn();
		}
	}
    return 0;
}
