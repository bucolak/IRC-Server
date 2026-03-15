#include <iostream>
#include <sstream>
#include <vector>

int main() {
    std::string veri = "PRIVMSG    Buket   :nasılsın";
    std::stringstream ss(veri); // Veriyi akışın içine koyduk
    std::string kelime;

    while (ss >> kelime) { // Her '>>' işlemi boşluğa kadar olan kısmı alır
        std::cout << "Bulunan: [" << kelime << "]" << std::endl;
        // Eğer kelime ":" ile başlıyorsa, özel mantığını burada kurabilirsin
    }
    std::cout<<kelime<<std::endl;
}