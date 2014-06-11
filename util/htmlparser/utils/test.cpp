#include "log.h"

using namespace EA_COMMON;

int main(int argc, char* argv[])
{
	if(!Init_Log("../etc/logtemplate.conf"))
		return -1;
	
	Debug("this is a test");
	Debug("This is %d a test",3);
	
	Info("This is log test");
	Info("This is %d log test",4);
	
	Warn("%s","http://121.241.240.97/MOSL/uploadedFiles/MOSL/Investor_Relations/Reporting/Quarterly_Reporting/Motilal%20Oswal%20Financial%20Services%20-%20Q2FY10%20Earnings%20update%20with%20Press%20release.pdf");
	Warn("this is %d warn",5);

	Fatal("this is fatal");
	Fatal("this is %d fatal",6);

	Error("this is error");
	Error("This is %d error",7);
    
	return 0;
}
 

