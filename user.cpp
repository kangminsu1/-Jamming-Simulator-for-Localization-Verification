// Window에서 사용하는 코드
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <time.h>

// 여기는 조정 필요
#define SERVER_PORT 12345 //server port
#define IPAddress "127.0.0.1" //server address
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int n;
char buf[50];
vector<double> latitude;
vector<double> longitude;
vector<double> height;

// 옵션------------------ 좌표 변환 값 담음
vector<double> changed_lat;
vector<double> changed_lon;
vector<double> changed_h;
// 옵션------------------ 좌표 변환 값 담음

// CSV 파일에 있는 경로 계획된 좌표 값 받아옴
void parshing(queue<char>& tt, int cnt) {
	string coord = "";

	while (!tt.empty()) {
		char c = tt.front();
		tt.pop();

		if (c == ',') {
			//latitude
			if (cnt == 0) {
				double lat = stod(coord);
				latitude.push_back(lat);
				break;
			}
			else if (cnt == 1) {
				double lon = stod(coord);
				longitude.push_back(lon);
				break;
			}
			else {
				double h = stof(coord);
				height.push_back(h);
				break;
			}
		}
		else {
			coord.push_back(c);
		}
	}
	coord.clear();
}

void read_file() {
	//GPS 파일 경로
	ifstream file("GPS_EX.csv", ios::in);
	if (file.fail()) { printf("file not exit! Try again..\n"); return; }

	queue<char> temp_data;
	while (file.good()) {
		char data = file.get();

		if (data == '\n') {
			temp_data.push(',');

			for (int i = 0; i < 3; i++) {
				parshing(temp_data, i);
			}

		}
		else {
			temp_data.push(data);
		}
	}

	file.close();
}

//Windows 환경에서의 통신. TCP/IP 연결하는 함수
SOCKET tcp_ip_connection() {
	WORD wVersionRequested;
	WSADATA		wsaData;
	SOCKADDR_IN servAddr; //Socket address information
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		cout << "WSAStartup error " << WSAGetLastError() << endl;
		WSACleanup();
		return false;
	}

	servAddr.sin_family = AF_INET; // address family Internet
	servAddr.sin_port = htons(SERVER_PORT); //Port to connect on
	inet_pton(AF_INET, IPAddress, &(servAddr.sin_addr.s_addr)); //Target IP


	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
	if (s == INVALID_SOCKET)
	{
		cout << "Socket error " << WSAGetLastError() << endl;
		WSACleanup();
		return false; //Couldn't create the socket
	}

	int x = bind(s, reinterpret_cast<SOCKADDR*>(&servAddr), sizeof(servAddr));
	if (x == SOCKET_ERROR)
	{
		cout << "Binding failed. Error code: " << WSAGetLastError() << endl;
		WSACleanup();
		return false; //Couldn't connect
	}

	return s;
}

SOCKET tcp_ip_connected(SOCKET s) {
	SOCKADDR_IN cliAddr;

	cout << "Waiting for client..." << endl;

	listen(s, 5);
	int xx = sizeof(cliAddr);
	SOCKET sock = accept(s, reinterpret_cast<SOCKADDR*>(&cliAddr), &xx);
	cout << "Connection established. New socket num is " << sock << endl;

	return sock;
}

//Windows 환경에서의 통신. TCP/IP 연결 끝내는 하는 함수
void tcp_disconnection(SOCKET binded) {
	closesocket(binded);
	WSACleanup();
}

// 수신되는 데이터 Split해서 데이터 파싱하는 함수
void split_llh(vector<double>& coord, char buf[], int max_len) {
	int cnt = 0;
	string data = "";

	for (int i = 2; i < max_len; i++) {
		if (buf[i] == ',') {
			coord[cnt] = stod(data);
			cnt++;
			data = "";
		}
		else {
			data.push_back(buf[i]);
		}
	}

}

// 옵션------------------ 변환된 좌표값 담는 함수
void contains(double lat, double lon, double h) {
	changed_lat.push_back(lat);
	changed_lon.push_back(lon);
	changed_h.push_back(h);
}

// 데이터 파싱 함수
string send_llh(int cnt, string head) {
	string header = head;
	string payload = "," + to_string(latitude[cnt]) + "," +
		to_string(longitude[cnt]) + "," + to_string(height[cnt]) + ',';
	header += payload;
	return header;
}

int main() {

	// 경로 파일 읽기
	read_file();

	// 데이터 들어온 것 확인
	//for (int i = 0; i < latitude.size(); i++) {
	//	printf("%4.8f %4.8f %4.8f\n", latitude[i], longitude[i], height[i]);
	//}

	SOCKET binded, sock;
	binded = tcp_ip_connection();
	sock = tcp_ip_connected(binded);

	string header;
	vector<double> coordinate(3, 0.0);

	cout << "sending START!";
	header = "1";
	send(sock, header.c_str(), header.length(), 0);

	int cnt = 0;
	while (true) {
		n = recv(sock, buf, 50, 0);
			
		if (n <= 0) { printf("Got nothing\n"); break; }
		buf[n] = 0; // make a string

		// ACK
		if (buf[0] == '1') {
			if (cnt < latitude.size()) {
				header = send_llh(cnt++, "2");
				send(sock, header.c_str(), header.length(), 0);
			}
			else {
				header = "3";
				send(sock, header.c_str(), header.length(), 0);
				cnt = 0;
			}
		}
		// 데이터 변환 끝났다는 소리
		else if (buf[0] == '3') {
			// ACK
			send(sock, "4", strlen("1"), 0);
		}
		else if (buf[0] == '4') {
			cout << "User's Received: " << buf << '\n';

			// 옵션-------변환된 데이터 저장하는 함수
			split_llh(coordinate, buf, n);
			contains(coordinate[0], coordinate[1], coordinate[2]);
			// 옵션-------변환된 데이터 저장하는 함수

			send(sock, "4", strlen("4"), 0);
		}
		// 할일 다 끝냄
		else if (buf[0] == '5') { break; }
	}

	// 예시 - GPS 데이터 들어오는 시뮬레이션
	// Python 모니터링이 필요하다면 이걸 사용--------
	WSADATA wsdata;
	WSAStartup(MAKEWORD(2, 2), &wsdata);
	sockaddr_in addrs;
	SOCKET udp_structure = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	addrs.sin_family = AF_INET;
	addrs.sin_port = htons(11111);
	inet_pton(AF_INET, IPAddress, &(addrs.sin_addr.s_addr));
	// Python 모니터링이 필요하다면 이걸 사용---------


	// 필수!--------------------
	// GTA에서 생성되는 GPS 데이터를 수신받는 문법 생성해야 함
	// https://ex3llo.tistory.com/48?category=683235 <-- 참조
	// 필수!--------------------

	cout << "\n***********Simulation START!***********\n";
	cnt = 0;
	while (true) {

		// 이 부분은 예시 코드이며, GTA의 GPS를 파싱할 시
		// 이 부분만 잘 읽어보고 원하는 대로 수정하면 됨
		// 간단하고 쉬운 코드라 쉽게 수정 가능함------
		if (cnt < latitude.size()) {
			printf("Original: lat: %llf, lon: %llf, h: %lf\n",
				latitude[cnt], longitude[cnt], height[cnt]);

			header = send_llh(cnt++, "0");
			send(sock, header.c_str(), header.length(), 0);
		}
		//-----------------------
		// 데이터를 다 보냈으면 -1을 보내 루프를 끝내는 문장
		else { send(sock, "-1", strlen("-1"), 0); break; }


		n = recv(sock, buf, 50, 0);
		if (n <= 0) { printf("Got nothing\n"); break; }
		buf[n] = 0; // make a string

		if (buf[0] == '1') {
			split_llh(coordinate, buf, n);
			printf("Rasberry: lat: %llf, lon: %llf, h: %lf\n",
				coordinate[0], coordinate[1], coordinate[2]);

			//python 코드에 보내는 문장: matplot plot용
			string pp = to_string(coordinate[0]) + "," + 
				to_string(coordinate[1]) + "," + to_string(coordinate[2]);

			sendto(udp_structure, pp.c_str(), pp.length(), 0, (sockaddr *)&addrs, sizeof(addrs));
		}

		Sleep(500);
	}

	tcp_disconnection(binded);
	return 0;
}