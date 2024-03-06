#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

unsigned int charset_to_int(string ch, size_t offset, bool flip = false);

struct Packet
{
    Packet()
    {
        ver = 0;
    }
    void assign_info(string pack_data)
    {
        ver = charset_to_int(pack_data.substr(12, 2), 2);
        string tmp = pack_data.substr(12, 3);
        return;
    }
    unsigned int ver;
};

unsigned int charset_to_int(string ch, size_t offset, bool flip = false)
{
    unsigned int sum = 0;
    if (flip)
    {
        for (int i = 0; i < offset; ++i)
        {
            sum = sum | (unsigned char)ch[i] << (i * 8);
        }
    }
    else
    {
        for (int i = 0; i < offset; ++i)
        {
            sum = sum | (unsigned char)ch[offset - 1 - i] << (i * 8);
        }
        return sum;
    }
}

Packet *resize(Packet *mem, unsigned size, unsigned new_size)
{
    Packet *new_mem = new Packet[new_size];
    std::copy(mem, mem + std::min(size, new_size), new_mem);
    delete[] mem;
    return new_mem;
}

int main()
{
    ifstream file;
    // file.open("C:\\HDD_slow\\docs\\c++\\preinterview\\packets.sig", std::ios::in | std::ios::binary);
    file.open("D:\\Docs\\c++\\preinterview\\packets.sig", std::ios::in | std::ios::binary);
    size_t size_len = 2;

    if (!file)
    {
        cout << "File didn't open\n\n";
        return -1;
    }

    string whole_file; // переменная для всего файла
    file.seekg(0, std::ios::end);
    whole_file.reserve(file.tellg());
    size_t whole_file_len = file.tellg();
    file.seekg(0, std::ios::beg);
    whole_file.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()); // присвоение строке символов из файла

    size_t pos = 0;
    size_t data_len = 0;

    size_t pack_arr_len = 50;
    size_t pack_arr_increment = 50;
    Packet *packs = new Packet[pack_arr_len];

    for (int i = 0; pos < whole_file_len; ++i)
    {
        data_len = (size_t)charset_to_int(whole_file.substr(pos, size_len), size_len, true);
        pos = pos + size_len;
        if (i == pack_arr_len)
        {
            packs = resize(packs, pack_arr_len, pack_arr_len + pack_arr_increment);
            pack_arr_len = pack_arr_len + pack_arr_increment;
        }
        packs[i].assign_info(whole_file.substr(pos, data_len));
        pos = pos + data_len;

        std::cout << packs[i].ver;
    }

    int count_ipv4 = 0;

    for (int i = 0; i < pack_arr_len; ++i)
    {
        if (packs[i].ver == 0)
        {
            packs = resize(packs, pack_arr_len, i);
            pack_arr_len = i;
            break;
        }
        if (packs[i].ver == 2048)
            ++count_ipv4;
    }

    cout << count_ipv4 << " " << pack_arr_len;

    delete[] packs;
    return 0;
}