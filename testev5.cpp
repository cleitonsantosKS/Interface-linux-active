#include <iostream>
#include <cstring>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace std;

bool isNetworkInterfaceUp(const string& interfaceName) {
    int sockfd;
    struct ifreq ifr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        cerr << "Erro ao abrir socket!" << endl;
        return false;
    }

    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
        cerr << "Erro ao obter status da interface!" << endl;
        close(sockfd);
        return false;
    }

    close(sockfd);

    if (ifr.ifr_flags & IFF_UP) {
        return true;
    }

    return false;
}

int main() {
    string interfaceName = "wlp3s0";
    int downCounter = 0;
    int maxDownAttempts = 7;
    int checkInterval = 1;

    while (true) {
        if (isNetworkInterfaceUp(interfaceName)) {
            cout << "A interface " << interfaceName << " está UP!" << endl;
            downCounter = 0;
        } else {
            cout << "A interface " << interfaceName << " está DOWN!" << endl;

            for (int i = 0; i < 10; i++) {
                sleep(checkInterval);

                if (isNetworkInterfaceUp(interfaceName)) {
                    cout << "A interface " << interfaceName << " voltou a ficar UP após " << i+1 << " tentativas." << endl;
                    downCounter = 0;
                    break;
                } else {
                    cout << "Tentativa " << i+1 << " de verificação: A interface " << interfaceName << " continua DOWN." << endl;
                    downCounter++;
                }
            }

            if (downCounter >= maxDownAttempts) {
                cout << "A interface " << interfaceName << " ficou DOWN por " << maxDownAttempts << " tentativas consecutivas." << endl;

                cout << "Desativando a interface " << interfaceName << "..." << endl;
                system(("sudo ifconfig " + interfaceName + " down").c_str());

                sleep(3);

                cout << "Ativando a interface " << interfaceName << "..." << endl;
                system("rfkill list");
                system("sudo rfkill unblock all");
                system("rfkill list");
                system("sudo nmcli radio wifi on");
                sleep(10);
                system("ip -br a");

                downCounter = 0;

                sleep(30);
            } else {
                downCounter++;
            }
        }

        sleep(3);
    }

    return 0;
}
