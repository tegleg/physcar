/*
	TestCar by tegleg
*/
using UnrealBuildTool;

public class TestCar : ModuleRules
{
    public TestCar(TargetInfo target)
    {
        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",			
				"RHI",
				"RenderCore",
                "ShaderCore",
                "UMG",
 "Slate",
 "SlateCore",
				"PhysX"
			}
		); 

//Private Paths
        PrivateIncludePaths.AddRange(new string[] { 
			"TestCar/Public",
			"TestCar/Private"
		});        

    }
}