#pragma once
 
#include "ModuleManager.h"
 
class TestCarImpl : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();

        bool IsThisNumber42(int32 num);
};