#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdint>
using namespace std;

unsigned int charset_to_int(vector<char> &pack_data, size_t pos, size_t end_pos, bool flip = false);

struct IP_adr
{
    IP_adr() { IP = 0; }
    IP_adr(vector<char> &ip_seq, size_t ip_start)
    {
        IP = charset_to_int(ip_seq, ip_start + 2, ip_start + 5);
    }
    uint32_t IP;
};

struct Packet
{
    Packet()
    {
        ver = 0;
    }
    Packet(vector<char> &pack_data, size_t pos, size_t end_pos)
    {
        ver = charset_to_int(pack_data, pos + 12, pos + 13);
        source = IP_adr(pack_data, pos + 6);
        destination = IP_adr(pack_data, pos);
    }
    unsigned int ver;
    IP_adr source;
    IP_adr destination;
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

    vector<char> whole_file;
    char current_ch = 0;
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