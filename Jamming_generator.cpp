#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <cmath>
#include <limits>
#include "communication.h"

//windows에서 데모 할 경우
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

//linux (라즈베리파이)에서 데모할 경우
//#include <arpa/inet.h>
//#include <sys/socket.h>

#define SERVER_PORT 12345 //server port
#define IPAddress "127.0.0.1" //server address
#define PI 3.14159265358979323846
using namespace std;

// 경로를 다 담고 좌표변환까지 하는 class
class waypoints {
private:
    vector<double> latitude;
    vector<double> longitude;
    vector<double> height;

    vector<double> changed_lat;
    vector<double> changed_lon;
    vector<double> changed_h;
    
    int index = 0;
    int max_size = 0;

public:
	waypoints() { printf("Waypoints Container Open!\n"); }
	~waypoints() { printf("Waypoints Container Close.\n"); }

    string header, payload;

	// header가 2일때, 데이터 축척
	void contains(double lat, double lon, double h) {
		latitude.push_back(lat);
		longitude.push_back(lon);
		height.push_back(h);
	}

    int length() { max_size = latitude.size(); return max_size; }
    void ind(int n) { index = n; }

    // 가장 가까운 좌표 index 구하기----------------------
    double convert_deg2rad(double deg) { return (deg * PI / 180); }
    double convert_rad2deg(double rad) { return (rad * 180 / PI); }
    double get_distance(double lat1, double lon1, double lat2, double lon2) {
        double theta, dist;
        if ((lat1 == lat2) && (lon1 == lon2)) return 0;
        else {
            theta = lon1 - lon2;
            dist = sin(convert_deg2rad(lat1)) * sin(convert_deg2rad(lat2)) +
                cos(convert_deg2rad(lat1)) * cos(convert_deg2rad(lat2))
                * cos(convert_deg2rad(theta));
            dist = acos(dist);
            dist = convert_rad2deg(dist);
            dist = dist * 60 * 1.1515;
            dist = dist * 1.609344 * 1000;
            return dist;
        }
    }

    pair<double, int> find_pos(double lat, double lon) {
        pair<double, int> min_dist = { DBL_MAX, 0 }; //distance, index
        double temp_dis;
        for (int i = 0; i < max_size; i++) {
            temp_dis = get_distance(latitude[i], longitude[i], lat, lon);
            if (min_dist.first > temp_dis) {
                min_dist.first = temp_dis;
                min_dist.second = i;
            }
        }

        return min_dist;
    }
    // 가장 가까운 좌표 index 구하기----------------------

    // 데이터 파싱 함수
    string send_llh(int cnt, string head, bool tf) {
        header = head;
        payload = "," + to_string(changed_lat[cnt]) + "," +
            to_string(changed_lon[cnt]) + "," + to_string(changed_h[cnt]) + ',';
        header += payload;

        if (tf == true) {
            printf("Send [lat: %llf, lon: %llf, h: %lf]\n", changed_lat[cnt]
                , changed_lon[cnt], changed_h[cnt]);
        }

        return header;
    }

    // 재밍 함수
    void jamming() {
        double mean, dev;
        cout << "\nYou Selected Jamming. Write mean and deviation. ex) mean: 0.0, deviation: 0.0001";
        cout << "\nmean: ";
        cin >> mean;
        cout << "deviation: ";
        cin >> dev;

        default_random_engine generator;
        normal_distribution<double> dist(mean, dev);
        cout << "Start making noise!...\n";

        double noise = 0.0;
        for (int i = 0; i < max_size; i++) {
            if (i >= index) {
                noise = dist(generator);
            }

            changed_lat.push_back(latitude[i] + noise);
            changed_lon.push_back(longitude[i] + noise);
            changed_h.push_back(height[i] + noise);
        }
    }

    // 스푸핑 함수
    void spoofing() {
        double dx, dy;
        cout << "\nYou selected Spoofing. Write increase or decrease (dx, dy). ex) dx: 1, dy: -4\n";
        cout << "dx: ";
        cin >> dx;
        cout << "dy: ";
        cin >> dy;

        double temp_dx = dx;
        double temp_dy = dy;

        cout << "Start making another coordination!...\n";

        for (int i = 0; i < max_size; i++) {
            if (i >= index) {
                changed_lat.push_back(latitude[i] + (temp_dx / 100000));
                changed_lon.push_back(longitude[i] + (temp_dy / 100000));
                temp_dx += dx;
                temp_dy += dy;
            }
            else {
                changed_lat.push_back(latitude[i]);
                changed_lon.push_back(longitude[i]);
            }
            changed_h.push_back(height[i]);
        }
    }

    // 미코닝 함수
    void meaconing() {
        int dt;
        cout << "\nYou selected Meaconing. Write finish meaconing time (x). ex) Time: 50\n";
        cout << "**Condition: x must higher then start jamming position -> [x > index]**\n";
        cout << "Time: ";
        cin >> dt;

        cout << "Start making Meaconing!...\n";

        for (int i = 0; i < latitude.size(); i++) {
            if (i >= index && i <= dt) {
                changed_lat.push_back(latitude[index]);
                changed_lon.push_back(longitude[index]);
                changed_h.push_back(height[index]);
            }
            else {
                changed_lat.push_back(latitude[i]);
                changed_lon.push_back(longitude[i]);
                changed_h.push_back(height[i]);
            }
        }
    }
};

//Windows 환경에서의 통신. TCP/IP 연결하는 함수
SOCKET tcp_ip_connection() {
    WORD		wVersionRequested;
    WSADATA		wsaData;
    SOCKADDR_IN target; //Socket address information
    SOCKET      s;
    int			err;

    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);

    if (err != 0) {
        printf("WSAStartup error %ld", WSAGetLastError());
        WSACleanup();
        return 0;
    }

    target.sin_family = AF_INET; // address family Internet
    target.sin_port = htons(SERVER_PORT); //Port to connect on
    inet_pton(AF_INET, IPAddress, &(target.sin_addr.s_addr)); //Target IP


    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (s == INVALID_SOCKET)
    {
        cout << "socket() error : " << WSAGetLastError() << endl;
        WSACleanup();
        return 0; //Couldn't create the socket
    }

    if (connect(s, reinterpret_cast<SOCKADDR*>(&target), sizeof(target)) == SOCKET_ERROR)
    {
        cout << "connect() error : " << WSAGetLastError() << endl;
        cout << "please server open first." << endl;
        WSACleanup();
        return 0; //Couldn't connect
    }

    return s;
}

//Windows 환경에서의 통신. TCP/IP 연결 끝내는 하는 함수
void tcp_disconnection(SOCKET binded) {
    closesocket(binded);
    WSACleanup();
}

// 수신되는 데이터 Split해서 데이터 파싱하는 함수
void split_llh(vector<double> &coord, char buf[], int max_len) {
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

int main() {
    waypoints *graph = new waypoints();

    // ---------------
    // 라즈베리파이용 tcp/ip를 추가적으로 작성해야함
    // Google에 "ubuntu c++ tcp ip" 쳐서 코드 작성 바람
	SOCKET  sock;
    sock = tcp_ip_connection();
    // 위 코드는 Window에서 작동하는 코드임
    // ---------------

    int n, index;
    int max_size = 0;
    char buf[50];
    vector<double> coordinate(3, 0.0);
    string header;
    int cnt = 0;

    while (true) {
        n = recv(sock, buf, 50, 0);

        // 만약에 들어오는 데이터가 없으면 루프 나감
        if (n <= 0) { printf("Got nothing\n"); break; }
        buf[n] = 0; // make a string

        // 1 == ACK <- 데이터 들어오는 거 확인
        if (buf[0] == '1') {
            send(sock, "1", strlen("1"), 0);
        }
        // 2 == 데이터 계속 받고 있음
        else if (buf[0] == '2') {
            cout << "Raspberry pi Received: " << buf << '\n';

            split_llh(coordinate, buf, n);
            // User로부터 수신된 좌표값 저장
            graph->contains(coordinate[0], coordinate[1], coordinate[2]);
            
            // 잘 받았다고 보냄
            send(sock, "1", strlen("1"), 0);
        }
        // 3 == 데이터 다 받음 --> 여기서부터 재밍 시작
        else if (buf[0] == '3') {
            max_size = graph->length();

            // 재밍 시나리오 고르는 Section
            int choose;
            cout << "\n\nChoose Jamming Scenario: 1. Jamming, 2. Spoofing, 3. Meaconing\n";
            cout << "Select-->";
            cin >> choose;

            cout << "\n\nChoose Start Jamming Position(index)\n";
            cout << "Select[0 ~ " << max_size - 1 << "] ->>";
            cin >> index;
            graph->ind(index);

            // 재밍, 스푸핑, 미코닝
            if (choose == 1) { graph->jamming(); }
            else if (choose == 2) { graph->spoofing(); }
            else if (choose == 3) {graph->meaconing();}

            cout << "Finished!\n";
            // 데이터 변환이 끝났다는 신호 보냄
            send(sock, "3", strlen("3"), 0);
        }
        // 4 == 변환된 데이터를 보냄 -> Raspberry to User
        else if (buf[0] == '4') {
            if (cnt < max_size - 1) {
                header = graph->send_llh(cnt++, "4", false);
                send(sock, header.c_str(), header.length(), 0);
            }
            else {
                header = "5"; // 5 = 다 보냄
                send(sock, header.c_str(), header.length(), 0);
                cnt = 0;
                break;
            }
        }
    }

    // GPS 값이 들어오면 변환한 값 찾아서 보냄
    while (true) {
        n = recv(sock, buf, 50, 0);

        if (n <= 0) { printf("Got nothing\n"); break; }
        buf[n] = 0; // make a string

        if (buf[0] == '0') {
            split_llh(coordinate, buf, n);
            printf("Recv [lat: %llf, lon: %llf, h: %lf]\n", coordinate[0], coordinate[1], coordinate[2]);

            //distance, index = 수신된 GPS 정보와 저장된 좌표 list를 비교하여 위치(index)추출 
            pair<double, int> min_dist = graph->find_pos(coordinate[0], coordinate[1]);
            header = graph->send_llh(min_dist.second, "1", true);
            send(sock, header.c_str(), header.length(), 0);
        }
        else if (buf[0] == '-1') {
            break;
        }
    }

    // 메모리 소멸
    delete graph;
    tcp_disconnection(sock);

	return 0;
}