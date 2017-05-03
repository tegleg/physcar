#pragma once

//#include "GameFramework/Pawn.h"
// Needed for custom physics
//#include "PhysicsPublic.h"

///////////////////////////)()()()()()()()())()()()()()()()()()()()()()()()()()
//
// should have a dynamic wheel struct array and apply force to a bone
// this would allow for n wheeled and articulated vehicles
//
// Start a new one!
//
//////////////////////////////()()()()()()()()()()()()()()()()()()()()()()()()()

#include "ATegCar.generated.h"

UCLASS()
class TESTCAR_API ATegCar : public APawn
{
	GENERATED_BODY()

private_subobject:
	/**  The main skeletal mesh associated with this Vehicle */
	DEPRECATED_FORGAME(4.6, "Mesh should not be accessed directly, please use GetMesh() function instead. Mesh will soon be private and your code will not compile.")
		UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	//UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//class UPoseableMeshComponent* VisibleMesh;

	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* Arrow0;
	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* Arrow1;
	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* Arrow2;
	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* Arrow3;
	//arrow array
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
		TArray<UArrowComponent*> ArrowArray;

	//wheel meshes
	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Wheel0;
	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Wheel1;
	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Wheel2;
	UPROPERTY(Category = VehicleSetup, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Wheel3;

	//wheel array
	UPROPERTY(BlueprintReadOnly, Category = VehicleSetup)
		TArray<UStaticMeshComponent*> WheelArray;

//private:
	/** audio component for engine sounds */
	UPROPERTY(Category = Effects, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineAC;

private:
	FCalculateCustomPhysics OnCalculateCustomPhysics;
	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);
	FHitResult Trace(FVector TraceStart, FVector TraceDirection);
	FBodyInstance *MainBodyInstance;

	UPROPERTY()
		TArray<float> PreviousPosition;

	UPROPERTY()
		FVector ArrowLocation;

public:
	// Sets default values for this pawn's properties
	ATegCar();

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* Camera;

	/** Name of the MeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
	static FName VehicleMeshComponentName;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	void SpawnSmokeEffect(int WheelIndex);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/** Handle pressing forwards */
	void MoveForward(float Val);

	/** Handle pressing right */
	void MoveRight(float Val);

	/** gear up */
	void ChangeUp();

	/** gear Down */
	void ChangeDown();

	FVector AddDrive(float DeltaTime, FBodyInstance* BodyInstance, FVector Loc, FVector Dir, int32 Index);
	FVector AddLatGrip(float DeltaTime, FBodyInstance* BodyInstance, FVector Loc, FVector Dir, int32 Index);

	float GetPowerToWheels(float DeltaTime, FBodyInstance* BodyInstance);

	void SpawnNewWheelEffect(int WheelIndex);
	/** update effects under wheels */
	void UpdateWheelEffects(float DeltaTime, int32 Index);

	//// vars /////////
	///////////////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
		float TraceLength = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
		float SpringValue = 800000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
		float MaxSpringValue = 1200000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
		float DamperValue = 1000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Suspension")
		TArray<bool> bOnGround;

	UPROPERTY(BlueprintReadOnly, Category = "Suspension")
		TArray<float> SpringLengthArray;

	UPROPERTY(BlueprintReadOnly, Category = "Suspension")
		FVector SpringLocation;
	//trace start locations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
		TArray<FVector> SpringTopLocation;
	UPROPERTY(BlueprintReadOnly, Category = "Suspension")
		TArray<FVector> SuspForceLocation;
	UPROPERTY(BlueprintReadOnly, Category = "Suspension")
		TArray<FVector> SpringForceArray;

	//anti-roll
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
		float AntiRollFront = 0.7f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
		float AntiRollBack = 0.6f;

	//engine --------------- 
	/** engine sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EngineSound")
		USoundCue* EngineSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EngineSound")
		float EnginePitchMax = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EngineSound")
		float EnginePitchIdle = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float EnginePower = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float EnginePowerScale = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float WastedPower = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float EngineIdleRPM = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float EngineMaxRPM = 7000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float AirResistance = 3.0f;
	
	
	UPROPERTY(EditDefaultsOnly, Category = "Engine")
		UCurveFloat* TorqueCurve;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		float Throttle = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		bool bClutchEngaged = false;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		float CurrentPower = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		float AvailablePower = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float RedLineRPM = 6000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float GearUpRPM = 5200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float GearDownRPM = 2000.0f;
	//mechanical/friction braking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float EngineBrake = 30.0f;
	//brake force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
		float BrakeForce = 50000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gears")
		TArray<float> Gears;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gears")
		int32 CurrentGear = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gears")
		float FinalGearRatio = 3.92;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gears")
		bool bAutomaticGears = true;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		float EngineRPM = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		float WheelRPM = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		bool bBraking = false;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		bool bInReverse = false;

	//for visual speed indicator
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		float SpeedKPH = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Engine")
		int32 SpeedKPH_int = 0;

	//wheel bones
	//UPROPERTY(EditAnywhere, Category = "Wheels")
	//	TArray<FName> BoneNames;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
		TArray<bool> bIsPowered;
	//steering
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		TArray<FVector> WheelCenterLocation;
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		TArray<FVector> TireHitLocation;
	//how fast visual wheels rotate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
		float RotationAmount = -1.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		TArray<float> CurrentWheelPitch;
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		float DeltaPitch = 0.0f;

	//for tire smoke ect
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		TArray<bool> bIsSliding;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
		float SlipThreshold = 140000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
		float SmokeKickIn = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
		float LongSlipThreshold = 100000.0f;
	//grip
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
			float Grip = 1.0f;
	//maximum weight/grip multiplier, cap for weight transfer formula. 
	//1 = no effect, >1 = capped to MaxGrip x default Grip
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
		float MaxGrip = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
		float MaxLatGrip = 2.0f;
	//unused
	//UPROPERTY(EditAnywhere, Category = "Wheels")
	//	float MaxLatGrip = 50000;
	//unused
	//UPROPERTY(EditAnywhere, Category = "Wheels")
	//	float MaxLongGrip = 50000;
	//
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		TArray<FVector> TireForceArray;
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		TArray<FVector> WheelForwardArray;
	UPROPERTY(BlueprintReadOnly, Category = "Wheels")
		TArray<FVector> WheelRightArray;

	//tire smoke
	UPROPERTY(EditAnywhere, Category = "Wheels")
		UParticleSystem* TireSmoke;
	UPROPERTY(EditAnywhere, Category = "Wheels")
		UParticleSystem* TireMarks;
	/** dust FX components */
	UPROPERTY(Transient)
		UParticleSystemComponent* DustPSC[4];

	/** skid sound loop */
	UPROPERTY(Category = Effects, EditDefaultsOnly)
		USoundCue* SkidSound;

	//max steer angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
		float SteerAngle = 45.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
		TArray<float> CurrentAngle;
	//lerp speed of wheel steering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
		float SteerSpeed = 3.0f;

	//wheel radius
	UPROPERTY(EditAnywhere, Category = "Wheels")
		float Radius = 32.0f;

	/** Returns Mesh subobject **/
	class USkeletalMeshComponent* GetMesh() const;

	//FORCEINLINE UPoseableMeshComponent* GetVisibleMesh() const {return VisibleMesh;}
	/** Returns SpringArm subobject **/
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

};

