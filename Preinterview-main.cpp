#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <iomanip>
using namespace std;

unsigned int charset_to_int(vector<char> &pack_data, size_t pos, size_t end_pos, bool flip = false);

uint8_t cutout_byte(uint64_t sequence, size_t place) { return uint8_t(sequence << (8 * place) >> 56); }

struct IP_adr
{
    IP_adr() { IP = 0; }
    IP_adr(vector<char> &ip_seq, int ip_start)
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
        transmissions_counter = 1;
    }
    Packet(vector<char> &pack_data, size_t pos /*, size_t end_pos*/)
    {
        ver = get_version(pack_data, pos);
        IP_sig = get_IP_signature(pack_data, pos);
        transmissions_counter = 1;
    }

    static uint64_t get_IP_signature(vector<char> &pack_data, size_t pos)
    {
        IP_adr source(pack_data, pos + 6);
        IP_adr destination(pack_data, pos);
        return (uint64_t)destination.IP << 32 | (uint64_t)source.IP;
    }

    string get_string_IPs()
    {
        string to_return = to_string(cutout_byte(IP_sig, 4));
        for (int i = 5; i < 8; ++i)
        {
            to_return = to_return + "." + to_string(cutout_byte(IP_sig, i));
        }
        to_return = to_return + " -> " + to_string(cutout_byte(IP_sig, 0));
        for (int i = 1; i < 4; ++i)
        {
            to_return = to_return + "." + to_string(cutout_byte(IP_sig, i));
        }
        return to_return;
    }

    static unsigned int get_version(vector<char> &pack_data, size_t pos) { return charset_to_int(pack_data, pos + 12, pos + 13); }

    unsigned int ver;
    uint64_t IP_sig;
    unsigned int transmissions_counter;
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

    vector<Packet> IPv4_packs; // вектор только для пакетов в4, согласно задаче. если надо хранить другие пакеты, масштабируется добавлением вектора other и записи туда при непрохождении проверки на в4
    vector<Packet>::iterator packs_iter;
    int count_ipv4 = 0;
    int count_other = 0;

    while (pos < whole_file.size())
    {
        bool v4_written_flag = false;
        data_len = (size_t)charset_to_int(whole_file, pos, pos + size_len - 1, true);
        pos = pos + size_len;
        packs_iter = IPv4_packs.begin();
        if (Packet::get_version(whole_file, pos) == 2048)
        {
            ++count_ipv4;
            for (; packs_iter != IPv4_packs.end(); packs_iter++)
            {
                if (packs_iter->IP_sig == Packet::get_IP_signature(whole_file, pos))
                {
                    packs_iter->transmissions_counter++;
                    v4_written_flag = true;
                    break;
                }
            }
            if (!v4_written_flag)
            {
                IPv4_packs.push_back(Packet(whole_file, pos));
            }
        }
        else
        {
            ++count_other;
        }
        pos = pos + data_len;
    }

    packs_iter = IPv4_packs.begin();
    cout << setw(40) << "Packets containing IPv4:"
         << "\t" << count_ipv4 << endl
         << setw(40) << "Packets without IPv4:"
         << "\t" << count_other << endl
         << endl;
    for (; packs_iter != IPv4_packs.end(); packs_iter++)
    {
        cout << setw(40) << packs_iter->get_string_IPs()
             << "\t" << packs_iter->transmissions_counter << endl;
    }

    return 0;
}