#include <fstream>
#ifdef _MSC_VER
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/shm.h>
#endif// _MSC_VER

const int IP_SIZE = 4;
const int OFFSET_SIZE = 3;
const int INDEX_RECORD_SIZE = IP_SIZE + OFFSET_SIZE;

class iplocater
{
public:
	iplocater(const std::string& ipFileName){
		std::fstream file;
		file.open(ipFileName, std::ios::binary | std::ios::in | std::ios::ate);
		if (!file.is_open())
		{
			throw std::exception("无法打开文件");
		}
		int size = file.tellg();
		file.seekg(std::ios::beg);


		m_ipbuff = (char*)malloc(sizeof(char) * size);
		file.read(m_ipbuff, size);

		memcpy((void*)&m_iFirstIndex, m_ipbuff, 4);
		memcpy((void*)&m_iLastIndex, m_ipbuff + 4, 4);
	}

	~iplocater(){
		free(this->m_ipbuff);
	}

	unsigned int ip2int(const char* ip) {
		return ntohl(inet_addr(ip));
	}

	void getIpAddress(const std::string& ip, char* area, char* sp)
	{
		this->getIpAddress(ip2int(ip.c_str()), area, sp);
	}

	void getIpAddress(const unsigned int& uip, char* area, char* sp)
	{
		int record_num = (m_iLastIndex - m_iFirstIndex) / INDEX_RECORD_SIZE + 1;
		int index = binary_find(uip, 0, record_num - 1);
		int index_offset = m_iFirstIndex + index * INDEX_RECORD_SIZE + 4;
		int addr_offset = 0;

		memcpy((void*)&addr_offset, m_ipbuff + index_offset, 3);

		int iRedirectMode = 0;
		char cRedirectMode = *(m_ipbuff + addr_offset + 4);

		int iCountryOffset = 0;
		if (cRedirectMode == REDIRECT_MODE_1)
		{
			memcpy((void*)&iCountryOffset, m_ipbuff + addr_offset + 5, 3);

			char byte = *(m_ipbuff + iCountryOffset);
			if (byte == REDIRECT_MODE_2)
			{
				int  iCtOf = 0;
				memcpy((void*)&iCtOf, m_ipbuff + iCountryOffset + 1, 3);
				readArea(iCtOf, area);
				readSp(iCountryOffset + 4, sp);
			}
			else
			{
				readArea(iCountryOffset, area);
				int newoffset = iCountryOffset + strlen(area) + 1;
				readSp(newoffset, sp);
			}
		}
		else if (cRedirectMode == REDIRECT_MODE_2)
		{
			memcpy((void*)&iCountryOffset, m_ipbuff + addr_offset + 5, 3);
			readArea(iCountryOffset, area);
			readSp(addr_offset + 8, sp);
		}
		else
		{
			readArea(addr_offset + 4, area);
			int newoffset = addr_offset + 4 + strlen(area) + 1;
			readSp(newoffset, sp);
		}
	}

	int binary_find(unsigned int ip, int left, int right) {
		if (right - left == 1)
		{
			return left;
		}
		else
		{
			int middle = (left + right) / 2;

			int offset = m_iFirstIndex + middle * INDEX_RECORD_SIZE;
			unsigned int new_ip = 0;
			memcpy((void*)&new_ip, m_ipbuff + offset, 4);

			if (ip >= new_ip) {
				return binary_find(ip, middle, right);
			}
			else {
				return binary_find(ip, left, middle);
			}
		}
	}

	int readArea(const int offset, char* sArea)
	{
		int len = 0;
		char sCountry[50] = { 0 };

		char ch = *(m_ipbuff + offset);
		while (ch != 0 && ch != EOF)
		{
			ch = *(m_ipbuff + offset + len);
			sCountry[len] = ch;
			len++;
		}
		strncpy(sArea, sCountry, len);

		return 0;
	}

	int readSp(const int offset, char* sSp)
	{
		char b = *(m_ipbuff + offset);

		if (b == REDIRECT_MODE_1 || b == REDIRECT_MODE_2)
		{
			int iSpOffset = 0;
			memcpy((void*)&iSpOffset, m_ipbuff + offset + 1, 3);

			if (iSpOffset)
			{
				readArea(iSpOffset, sSp);
			}
			else
			{
				strncpy(sSp, "Unkown", 6);
			}
		}
		else
		{
			readArea(offset, sSp);
		}

		return 0;
	}

private:
	unsigned int m_iFirstIndex;		//第一个索引
	unsigned int m_iLastIndex;		//最后一个索引
	char* m_ipbuff;
	enum
	{
		REDIRECT_MODE_1 = 0x01,
		REDIRECT_MODE_2 = 0x02
	};
};