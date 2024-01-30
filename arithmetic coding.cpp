#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>

using namespace std;



void CodeText(string & S , unordered_map<char, int> & freq, vector<unsigned short> & list, vector<char> & simvol, vector<unsigned short> & b){
    ofstream fout("encode.txt", std::ios_base::binary); 
    string result = "";
    if(fout.is_open()){
        //for(int i = 0; i < list.size(); i++)  out << simvol[i] << " : " << list[i] << endl;

        unsigned short l = 0, h = 65535, l_prev, h_prev, delitel = b[(int)b.size() - 1];
        unsigned short First_qtr = (h + 1) / 4; 
        unsigned short Half = First_qtr * 2; 
        unsigned short Third_qtr = First_qtr * 3;
        unsigned short bits_to_follow = 0; 
        unsigned short byte_2_size = 0, byte_2 = 0;
        for (char ch : S) { 
            unsigned short j = freq[ch];
            l_prev = l;
            h_prev = h;
            l = l_prev + b[j-1] * (h_prev - l_prev + 1) / delitel;
            h = l_prev + b[j] * (h_prev - l_prev + 1) / delitel - 1;
             for (;;) {
                if (h < Half) {
                    byte_2 = byte_2 << 1;
                    byte_2_size++;
                    if (byte_2_size == 16) {
                        fout.write((char*)&byte_2, 2);
                        byte_2 = 0;
                        byte_2_size = 0;
                    }
                    for (;bits_to_follow > 0; bits_to_follow--) {
                        byte_2 = byte_2 << 1 | 1;
                        byte_2_size++;
                        if (byte_2_size == 16) {
                            fout.write((char*)&byte_2, 2);
                            byte_2 = 0;
                            byte_2_size = 0;
                        }
                    }
                }
                else if (l >= Half) {
                    byte_2 = byte_2 << 1 | 1;
                    byte_2_size++;
                    if (byte_2_size == 16) {
                        fout.write((char*)&byte_2, 2);
                        byte_2 = 0;
                        byte_2_size = 0;
                    }
                    for (; bits_to_follow > 0; bits_to_follow--) {
                        byte_2 = byte_2 << 1;
                        byte_2_size++;
                        if (byte_2_size == 16) {
                            fout.write((char*)&byte_2, 2);
                            byte_2 = 0;
                            byte_2_size = 0;
                        }
                    }
                    l -= Half;
                    h -= Half;
                }
                else if (l >= First_qtr && h < Third_qtr) {
                    bits_to_follow++;
                    l -= First_qtr;
                    h -= First_qtr;
                }
                else break;
                l <<= 1;
                h <<= 1;
                h++;
            }
        }
        if (byte_2_size) { 
            byte_2 = byte_2 << (16 - byte_2_size);
            fout.write((char*)&byte_2, 2);
        }   

    fout.put('\n');

    
    }
    
    fout.close();

    ofstream Fout("encode.txt", std::ios::app);
    if(Fout.is_open()){
        for(int i = 0; i < list.size(); i++) Fout << simvol[i] << " : " << list[i] << endl;
    }
    Fout.close();
}

void DecodeText(unordered_map<char, int> & freq, vector<unsigned short> & list, vector<char> & simvol, vector<unsigned short> & b){
    ofstream out("decode.txt");
    string result = "";
    ifstream fin("encode.txt", ios_base::binary);

    if(out.is_open()){

        unsigned short l = 0, h = 65535, l_prev, h_prev, delitel = b[(int)b.size() - 1], DataLength = b[(int)b.size() - 1];
        unsigned short First_qtr = (h + 1) / 4;
        unsigned short Half = First_qtr * 2; 
        unsigned short Third_qtr = First_qtr * 3;
        unsigned short value, freq, buf, buf_size = 0;
        int i = 0;
        fin.read((char*)&value, 2);
       
        for (unsigned short i = 1, j; i <= DataLength; i++) {
            l_prev = l;
            h_prev = h;
            freq = ((value - l_prev + 1) * delitel - 1) / (h_prev - l_prev + 1);

            
            for (j = 1; b[j] <= freq; j++); 
            l = l_prev + b[j - 1] * (h_prev - l_prev + 1) / delitel;
            h = l_prev + b[j] * (h_prev - l_prev + 1) / delitel - 1;
            for (;;) {
                if (h < Half) {}
                else if (l >= Half) {
                    value -= Half;
                    l -= Half;
                    h -= Half;
                }
                else if (l >= First_qtr && h < Third_qtr) {
                    l -= First_qtr;
                    h -= First_qtr;
                    value -= First_qtr;
                }
                else break;
                l <<= 1;
                h <<= 1;
                h++;
                if(buf_size) {
                    value <<= 1;
                    value |= (buf >> (--buf_size) & 1);
                }
                else {
                    if (!(fin.read((char*)&buf, 2))) { buf = 0; }
                    buf_size = 16;
                    value <<= 1;
                    value |= (buf >> (--buf_size) & 1);
                }
            }
             if(i + 1 <= DataLength) out <<static_cast<unsigned char>(simvol[j-1]);
             else  out <<static_cast<unsigned char>(simvol[j]);
        }
    }
    out.close();
}

int main(){


    string S = "";

    unordered_map<char, int> freq;
    ifstream in("text.txt");

    char a;
    while (in.get(a)) {
            freq[a]++;
            S += a;
        }
    in.close();

    vector<unsigned short> list; 
    vector<char> simvol;

    for (auto pair : freq) { 
        list.push_back(0); 
        simvol.push_back(0);
        int j = (int)list.size() - 1;  
        while (j > 0 && pair.second >= list[j - 1]) {
            list[j] = list[j - 1];
             simvol[j] = simvol[j - 1];
            --j;
        }
        list[j] = pair.second;
        simvol[j] = pair.first;
    }

    vector<unsigned short> b((int)simvol.size() + 1 );
    b[0] = 0;
    int x = 0;
        for (int i = 0 ; i < (int)simvol.size(); i++) {
            freq[simvol[i]] = i + 1;
            x += list[i];
            b[i+1] = x;
        }


    bool coding = false;
    bool flag = true;

    while(flag){
        cout << "что вы хотите сделать :" << endl;
        cout << "1) закодировать текст из файла" << endl;
        cout << "2) декадировать текст" << endl;
        cout << "3) выход" << endl;

        int z;

        cin >> z;

        if(z == 1){
            CodeText(S, freq, list, simvol, b);
            coding = true;
        }

        else if (z == 2){
            if(!coding) cout << "в начале закодируйте текст" << endl;
            else DecodeText(freq, list, simvol, b);
        }

        else if(z == 3) flag = false;
     
    }
    




    return 0;


    
}