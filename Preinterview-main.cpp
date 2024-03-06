#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

unsigned int charset_to_int(vector<char> &pack_data, size_t pos, size_t end_pos, bool flip = false);

// struct IP
// {
//     IP(){}
//     IP(string )
// };

struct Packet
{
    Packet()
    {
        ver = 0;
    }
    Packet(vector<char> &pack_data, size_t pos, size_t end_pos)
    {
        ver = charset_to_int(pack_data, pos + 12, pos + 13);
    }
    unsigned int ver;
};

unsigned int charset_to_int(vector<char> &pack_data, size_t pos, size_t end_pos, bool flip /*= false*/)
{
    unsigned int sum = 0;
    if (flip)
    {
        for (int i = end_pos; i >= (int)pos; --i)
        {
            char tmp = pack_data[i];
            sum = sum | (unsigned char)pack_data[i] << ((i - pos) * 8);
        }
    }
    else
    {
        for (int i = pos; i <= (int)end_pos; ++i)
        {
            sum = sum | (unsigned char)pack_data[i] << ((end_pos - i) * 8);
        }
    }
    return sum;
}

int main()
{
    ifstream file;
    // file.open("C:\\HDD_slow\\docs\\c++\\preinterview\\packets.sig", std::ios::in | std::ios::binary);
    file.open("D:\\Docs\\c++\\preinterview\\packets.sig", ios::in | ios::binary);
    size_t size_len = 2;

    if (!file)
    {
        cout << "File didn't open\n\n";
        return -1;
    }

    // string whole_file; // переменная для всего файла
    // file.seekg(0, ios::end);
    // whole_file.reserve(file.tellg());
    // size_t whole_file_len = file.tellg();
    // file.seekg(0, ios::beg);
    // whole_file.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>()); // присвоение строке символов из файла
    vector<char> whole_file;
    char current_ch = 0;
    // while (file >> current_ch)
    // {
    //     whole_file.push_back(current_ch);
    // }
    file.seekg(0, std::ios::end);
    std::streampos length(file.tellg());
    if (length)
    {
        file.seekg(0, std::ios::beg);
        whole_file.resize(static_cast<std::size_t>(length));
        file.read(&whole_file.front(), static_cast<std::size_t>(length));
    }

    size_t pos = 0;
    size_t data_len = 0;

    vector<Packet> packs;

    while (pos < whole_file.size())
    {
        data_len = (size_t)charset_to_int(whole_file, pos, pos + size_len - 1, true);
        pos = pos + size_len;
        packs.push_back(Packet(whole_file, pos, pos + data_len - 1));
        pos = pos + data_len;
    }

    int count_ipv4 = 0;

    for (int i = 0; i < packs.size(); ++i)
    {
        if (packs[i].ver == 2048)
            ++count_ipv4;
    }

    cout << count_ipv4 << " " << packs.size();

    return 0;
}