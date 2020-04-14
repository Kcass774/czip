#include "czip.hpp"

int main()
{
	iplocater* iploc = new iplocater("qqwry.Dat");
	char ip[] = "183.60.111.96";

	char area[50] = { 0 };
	char sp[50] = { 0 };
	iploc->getIpAddress(ip, area, sp);

	printf("area:%s|sp:%s\n", area, sp);
	delete iploc;
	return 0;
}