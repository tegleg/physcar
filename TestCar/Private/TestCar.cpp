#include "TestCarPrivatePCH.h"
 
#include "TestCar.h"
 
void TestCarImpl::StartupModule()
{

}
 
void TestCarImpl::ShutdownModule()
{
}

bool TestCarImpl::IsThisNumber42(int32 num)
{
	return num == 42;
}
 
IMPLEMENT_MODULE(TestCarImpl, Module)