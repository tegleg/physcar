#include "TestCarPrivatePCH.h"

FName ATegCar::VehicleMeshComponentName(TEXT("VehicleMesh"));

// Sets default values
ATegCar::ATegCar()
{

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(VehicleMeshComponentName);
	GetMesh()->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	//GetMesh()->SetVisibility(false , false);
	GetMesh()->BodyInstance.bSimulatePhysics = true;
	GetMesh()->BodyInstance.bNotifyRigidBodyCollision = true;
	GetMesh()->BodyInstance.bUseCCD = true;
	GetMesh()->bBlendPhysics = true;
	GetMesh()->bGenerateOverlapEvents = true;
//	GetMesh()->bCanEverAffectNavigation = false;
	RootComponent = GetMesh();

	//camera stuff //
	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 50.f);
	SpringArm->SetRelativeRotation(FRotator(-12.f, 0.f, 0.f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 750.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	//arrows used as trace start locations
	SpringTopLocation.Add(FVector(120.0f, 90.0f, 20.0f));
	SpringTopLocation.Add(FVector(120.0f, -90.0f, 20.0f));
	SpringTopLocation.Add(FVector(-120.0f, 90.0f, 20.0f));
	SpringTopLocation.Add(FVector(-120.0f, -90.0f, 20.0f));

	Arrow0 = CreateDefaultSubobject<UArrowComponent>(TEXT("arrow0"));
	Arrow0->SetupAttachment(RootComponent);
	Arrow0->SetRelativeLocation(SpringTopLocation[0]);

	Arrow1 = CreateDefaultSubobject<UArrowComponent>(TEXT("arrow1"));
	Arrow1->SetupAttachment(RootComponent);
	Arrow1->SetRelativeLocation(SpringTopLocation[1]);

	Arrow2 = CreateDefaultSubobject<UArrowComponent>(TEXT("arrow2"));
	Arrow2->SetupAttachment(RootComponent);
	Arrow2->SetRelativeLocation(SpringTopLocation[2]);

	Arrow3 = CreateDefaultSubobject<UArrowComponent>(TEXT("arrow3"));
	Arrow3->SetupAttachment(RootComponent);
	Arrow3->SetRelativeLocation(SpringTopLocation[3]);

	ArrowArray.Emplace(Arrow0);
	ArrowArray.Emplace(Arrow1);
	ArrowArray.Emplace(Arrow2);
	ArrowArray.Emplace(Arrow3);

	//wheel meshes
	Wheel0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel0"));
	Wheel0->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Wheel0->SetupAttachment(RootComponent);
	WheelArray.Emplace(Wheel0);

	Wheel1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel1"));
	Wheel1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Wheel1->SetupAttachment(RootComponent);
	WheelArray.Emplace(Wheel1);

	Wheel2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel2"));
	Wheel2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Wheel2->SetupAttachment(RootComponent);
	WheelArray.Emplace(Wheel2);

	Wheel3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel3"));
	Wheel3->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Wheel3->SetupAttachment(RootComponent);
	WheelArray.Emplace(Wheel3);

	//engine sound
	EngineAC = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	//EngineAC->SetSound(EngineSound);
	EngineAC->SetupAttachment(GetMesh());

	WheelCenterLocation.Init(FVector(0.0f, 0.0f, 0.0f), 4);
	TireHitLocation.Init(FVector(0.0f, 0.0f, 0.0f), 4);
	SpringForceArray.Init(FVector(0.0f, 0.0f, 0.0f), 4); 
	TireForceArray.Init(FVector(0.0f, 0.0f, 0.0f), 4);
	WheelForwardArray.Init(FVector(0.0f, 0.0f, 0.0f), 4);
	WheelRightArray.Init(FVector(0.0f, 0.0f, 0.0f), 4);
	SuspForceLocation.Init(FVector(0.0f, 0.0f, 0.0f), 4);

	SpringLengthArray.Init(0.0f, 4);

	PreviousPosition.Init(0.0f, 4);

	bOnGround.Init(false, 4);
	bIsPowered.Init(true, 4);
	bIsSliding.Init(false, 4);

	CurrentAngle.Init(0.0f, 4);

	CurrentWheelPitch.Init(0.0f, 4);

	//gear ratios
	Gears.Emplace(-2.90);//reverse
	Gears.Emplace(2.66);//1st
	Gears.Emplace(1.78);
	Gears.Emplace(1.30);
	Gears.Emplace(1.0);
	Gears.Emplace(0.74);//5th
	

	// Bind function delegate
	OnCalculateCustomPhysics.BindUObject(this, &ATegCar::CustomPhysics);

}

// Called when the game starts or when spawned
void ATegCar::BeginPlay()
{
	Super::BeginPlay();

	if (EngineAC)
	{
		EngineAC->SetSound(EngineSound);
		EngineAC->Play();
	}

	if (GetMesh() != NULL) {
		MainBodyInstance = GetMesh()->GetBodyInstance();
	}
}

// Called every frame
void ATegCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WheelArray.Num() == 0) { return; }

	//work out wheel center locations from visiblemesh instead of physics body to avoid wheel lag
		FTransform BodyTransform = Mesh->GetComponentTransform();
		FVector BodyLocation = BodyTransform.GetLocation();

		FVector BodyForwardVector = BodyTransform.GetUnitAxis(EAxis::X);
		FVector BodyRightVector = BodyTransform.GetUnitAxis(EAxis::Y);
		FVector BodyUpVector = BodyTransform.GetUnitAxis(EAxis::Z);

	for (int32 b = 0; b < WheelArray.Num(); b++)
	{

		FVector SuspensionLocation = BodyLocation + BodyForwardVector * SpringTopLocation[b].X + BodyRightVector * SpringTopLocation[b].Y + BodyUpVector * SpringTopLocation[b].Z;
		FVector TempVel = MainBodyInstance->GetUnrealWorldVelocityAtPoint(SuspensionLocation);

		//set wheel location
		WheelCenterLocation[b] = SuspensionLocation - (BodyUpVector * (SpringLengthArray[b] - Radius));

		FVector WheelForward = BodyForwardVector.RotateAngleAxis(CurrentAngle[b], BodyUpVector);		

		float ForwardSpeed = FVector::DotProduct(WheelForward, TempVel);
		
		DeltaPitch = -ForwardSpeed *RotationAmount*DeltaTime;
		CurrentWheelPitch[b] -= DeltaPitch;

		if (WheelArray[b] != NULL) {
			//rotate/locate wheel meshes
			WheelArray[b]->SetWorldLocation(WheelCenterLocation[b]);
			WheelArray[b]->SetRelativeRotation(FRotator(CurrentWheelPitch[b], CurrentAngle[b], 0.0f));
		}

		//tire smoke
		if (bIsSliding[b]) {
			//spawn smoke particle
			SpawnSmokeEffect(b);
			UpdateWheelEffects(DeltaTime, b);
			//reset sliding
			bIsSliding[b] = false;
		}
		else {
			if (DustPSC[b]) {
				DustPSC[b]->SetActive(false);
				DustPSC[b]->bAutoDestroy = true;
			}
			
		}


		/**DrawDebugLine(
		GetWorld(),
		SuspensionLocation,
		WheelCenterLocation[b],
		FColor(255, 0, 0),
		false, -1, 0,
		12.333
		);*/
	}

	

	// Add custom physics on MainBodyMesh
	if (MainBodyInstance != NULL) {
		MainBodyInstance->AddCustomPhysics(OnCalculateCustomPhysics);
	}

	//engine sound pitch
	if (EngineAC)
	{
		float EnginePitch = 0.0f;
		EnginePitch += ((EngineRPM / EngineMaxRPM)* (EnginePitchMax - EnginePitchIdle)) + EnginePitchIdle;
		EnginePitch = FMath::Clamp(EnginePitch, EnginePitchIdle, EnginePitchMax);
		EngineAC->SetPitchMultiplier(EnginePitch);
	}

	

}

void ATegCar::SpawnSmokeEffect(int WheelIndex)
{
	UParticleSystemComponent* NewParticle = NewObject<UParticleSystemComponent>(this);
	NewParticle->SetWorldLocation(TireHitLocation[WheelIndex]);
	NewParticle->bAutoActivate = true;
	NewParticle->bAutoDestroy = false;
	NewParticle->RegisterComponentWithWorld(GetWorld());
	NewParticle->SetTemplate(TireSmoke);

}

//for tire marks
void ATegCar::UpdateWheelEffects(float DeltaTime, int32 Index)
{
	if (TireMarks) {

				UParticleSystem* WheelFX = TireMarks;

				const bool bIsActive = DustPSC[Index] != NULL && !DustPSC[Index]->bWasDeactivated && !DustPSC[Index]->bWasCompleted;
				UParticleSystem* CurrentFX = DustPSC[Index] != NULL ? DustPSC[Index]->Template : NULL;
				if (WheelFX != NULL && (CurrentFX != WheelFX || !bIsActive))
				{
					if (DustPSC[Index] == NULL || !DustPSC[Index]->bWasDeactivated)
					{
						if (DustPSC[Index] != NULL)
						{
							DustPSC[Index]->SetActive(false);
							DustPSC[Index]->bAutoDestroy = true;
						}
						SpawnNewWheelEffect(Index);
						
					}
						DustPSC[Index]->SetTemplate(WheelFX);
						DustPSC[Index]->ActivateSystem();

				}
				else if (WheelFX == NULL && bIsActive)
				{
					DustPSC[Index]->SetActive(false);
				}

				if (DustPSC[Index]) {
					//set position
					DustPSC[Index]->SetWorldLocation(TireHitLocation[Index]);
				}
				
		
	}
}

void ATegCar::SpawnNewWheelEffect(int WheelIndex)
{
	DustPSC[WheelIndex] = NewObject<UParticleSystemComponent>(this);
	DustPSC[WheelIndex]->bAutoActivate = true;
	DustPSC[WheelIndex]->bAutoDestroy = false;
	DustPSC[WheelIndex]->RegisterComponentWithWorld(GetWorld());
	DustPSC[WheelIndex]->SetupAttachment(GetMesh());
}

// Called to bind functionality to input
void ATegCar::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATegCar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATegCar::MoveRight);
	PlayerInputComponent->BindAction("ChangeUp", IE_Pressed, this, &ATegCar::ChangeUp);
	PlayerInputComponent->BindAction("ChangeDown", IE_Pressed, this, &ATegCar::ChangeDown);

}

void ATegCar::MoveForward(float Val)
{
	Throttle = Val;
}

void ATegCar::MoveRight(float Val)
{
	//for substep calculations
	CurrentAngle[0] = FMath::Lerp(CurrentAngle[0], SteerAngle * Val, SteerSpeed * GetWorld()->DeltaTimeSeconds);
	CurrentAngle[1] = FMath::Lerp(CurrentAngle[1], SteerAngle * Val, SteerSpeed * GetWorld()->DeltaTimeSeconds);

}


// Called every substep for selected body instance
void ATegCar::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	//get physics body location
	FTransform BodyTransform = BodyInstance->GetUnrealWorldTransform();
	FVector BodyLocation = BodyTransform.GetLocation();

	//get direction vectors
	FVector BodyForwardVector = BodyTransform.GetUnitAxis(EAxis::X);
	FVector BodyRightVector = BodyTransform.GetUnitAxis(EAxis::Y);
	FVector BodyUpVector = BodyTransform.GetUnitAxis(EAxis::Z);

	//forward speed
	FVector Vel = BodyInstance->GetUnrealWorldVelocity();
	SpeedKPH = FMath::Abs(FVector::DotProduct(BodyForwardVector, Vel)) * 0.036f;
	SpeedKPH_int = FMath::FloorToInt(SpeedKPH);

	//add air resistance force
	FVector DragForce = -Vel * SpeedKPH * AirResistance;
	BodyInstance->AddImpulseAtPosition(DragForce * DeltaTime, BodyLocation);

	//if we got wheels	
	if (WheelArray.Num()>0) {

		//trace for each wheel	
		for (int32 Index = 0; Index < WheelArray.Num(); Index++)
		{
			
		//wheel direction vectors
		FVector WheelForward = BodyForwardVector.RotateAngleAxis(CurrentAngle[Index], BodyUpVector);
		FVector WheelRight = BodyRightVector.RotateAngleAxis(CurrentAngle[Index], BodyUpVector);

		FVector SuspensionLocation = BodyLocation + BodyForwardVector * SpringTopLocation[Index].X + BodyRightVector * SpringTopLocation[Index].Y + BodyUpVector * SpringTopLocation[Index].Z;
		//set location for susp force
		SuspForceLocation[Index] = SuspensionLocation;

		//trace
		FHitResult Hit = Trace(SuspensionLocation, -BodyUpVector);	
		
		if (Hit.bBlockingHit) {

			SpringLocation = Hit.ImpactPoint;

			float SpringPosition = (Hit.Location - SuspensionLocation).Size();
			SpringLengthArray[Index] = SpringPosition;

			// If previously on air, set previous position to current position
			if (!bOnGround[Index]) {
				PreviousPosition[Index] = SpringPosition;
			}
			float DamperVelocity = (SpringPosition - PreviousPosition[Index]) / DeltaTime;
			PreviousPosition[Index] = SpringPosition;
			bOnGround[Index] = true;

			// Calculate spring force
			float SpringForce = (1 - (SpringPosition / TraceLength)) * SpringValue;

			// Apply damper force
			SpringForce -= DamperValue * DamperVelocity;

			FVector TotalForce = Hit.ImpactNormal * FVector::DotProduct(Hit.ImpactNormal, SpringForce * BodyUpVector); //SpringForce * BodyUpVector;//// Hit.ImpactNormal;
			//set total force for this spring
			SpringForceArray[Index] = TotalForce;

			//for force calcs and tire smoke
			TireHitLocation[Index] = SpringLocation;

		}
		else {
			bOnGround[Index] = false;
			SpringLengthArray[Index] = TraceLength;

		}
		
		//grip and drive
		if (bOnGround[Index]) {
			//multiply grip by spring length, compressed = more grip ...
			float GripMultiplier = FMath::Min(TraceLength/SpringLengthArray[Index], MaxGrip);

			//add tire forces
			//lat grip
			FVector TireForces = AddLatGrip(DeltaTime, BodyInstance, SpringLocation, WheelRight, Index);
			//engine/wheel torque
			if (bIsPowered[Index]) {
				TireForces += AddDrive(DeltaTime, BodyInstance, SpringLocation, WheelForward, Index);
			}
			
			 
			TireForces *= GripMultiplier;
			BodyInstance->AddImpulseAtPosition(TireForces * DeltaTime, SpringLocation);
		}
		
		
		//~~~~~~~~~~~~~~~~~~~~~~
		}//end for()

		
		
		//add in anti-roll, take a bit of force from the longest spring and add it to the shotrest
		//we can do this with the force because RB local -z will be the same for all wheels
		//anti roll front
		FVector AntiRollForceF = FVector();
		if (bOnGround[0] && bOnGround[1]) {
			AntiRollForceF = (SpringForceArray[0] - SpringForceArray[1]) * AntiRollFront;
			AntiRollForceF.X = FMath::Clamp(AntiRollForceF.X, -MaxSpringValue, MaxSpringValue);
			AntiRollForceF.Y = FMath::Clamp(AntiRollForceF.Y, -MaxSpringValue, MaxSpringValue);
			AntiRollForceF.Z = FMath::Clamp(AntiRollForceF.Z, -MaxSpringValue, MaxSpringValue);
		}

		//anti roll back
		FVector AntiRollForceB = FVector();
		if (bOnGround[2] && bOnGround[3]) {
			AntiRollForceB = (SpringForceArray[2] - SpringForceArray[3]) * AntiRollBack;
			AntiRollForceB.X = FMath::Clamp(AntiRollForceB.X, -MaxSpringValue, MaxSpringValue);
			AntiRollForceB.Y = FMath::Clamp(AntiRollForceB.Y, -MaxSpringValue, MaxSpringValue);
			AntiRollForceB.Z = FMath::Clamp(AntiRollForceB.Z, -MaxSpringValue, MaxSpringValue);
		}
		
		//suspension springs
		//front
		if (bOnGround[0]) {
			SpringForceArray[0] += AntiRollForceF;
			BodyInstance->AddImpulseAtPosition(SpringForceArray[0] * DeltaTime, SuspForceLocation[0]);
		}
		if (bOnGround[1]) {
			SpringForceArray[1] -= AntiRollForceF;
			BodyInstance->AddImpulseAtPosition(SpringForceArray[1]  * DeltaTime, SuspForceLocation[1]);
		}
		//back
		if (bOnGround[2]) {
			SpringForceArray[2] += AntiRollForceB;
			BodyInstance->AddImpulseAtPosition(SpringForceArray[2] * DeltaTime, SuspForceLocation[2]);
		}
		if (bOnGround[3]) {
			SpringForceArray[3] -= AntiRollForceB;
			BodyInstance->AddImpulseAtPosition(SpringForceArray[3] * DeltaTime, SuspForceLocation[3]);
		}
	}
}

FHitResult ATegCar::Trace(FVector TraceStart, FVector TraceDirection) {
	//should change to shape trace, look into that................ <------------------------------------
	FHitResult Hit(ForceInit);
	FCollisionQueryParams TraceParams(true);
	FCollisionObjectQueryParams ObjectParams = ECC_WorldStatic | ECC_WorldDynamic; // object types that trace hits
	TraceParams.bTraceAsyncScene = true;
	//ignore self
	TraceParams.AddIgnoredActor(this);
	TraceParams.bReturnPhysicalMaterial = false;
	FVector TraceEnd = TraceStart + (TraceDirection * TraceLength);

	GetWorld()->LineTraceSingleByObjectType(Hit, TraceStart, TraceEnd, ObjectParams, TraceParams);

	return Hit;
}

void ATegCar::ChangeUp()
{
	if (CurrentGear < (Gears.Num()-1)) {
		CurrentGear++;
	}
}

void ATegCar::ChangeDown()
{
	if (CurrentGear > 0) {
		CurrentGear--;
	}
}

//force to wheels from 'engine'
FVector ATegCar::AddDrive(float DeltaTime, FBodyInstance* BodyInstance, FVector Loc, FVector Dir, int32 Index) {

	FVector TempVel = BodyInstance->GetUnrealWorldVelocityAtPoint(Loc);
	FTransform BodyTransform = BodyInstance->GetUnrealWorldTransform();
	FVector BodyLocation = BodyTransform.GetLocation();

	FVector BodyForwardVector = BodyTransform.GetUnitAxis(EAxis::X);
	FVector BodyUpVector = BodyTransform.GetUnitAxis(EAxis::Z);

	FVector WheelForward = BodyForwardVector.RotateAngleAxis(CurrentAngle[Index], BodyUpVector);

	

	//reverse
	if (FVector::DotProduct(WheelForward, TempVel) < 10.0f) {
		bInReverse = true;
	}
	else {
		bInReverse = false;
	}
	
	//braking
	if (Throttle > 0.0) {
		if (bInReverse) {
			bBraking = true;
		}
		else {
			bBraking = false;
		}
	}
	else if (Throttle < 0.0) {
		if (bInReverse) {
			bBraking = false;
		}
		else {
			bBraking = true;
		}
	}
	else {
		bBraking = false;
	}

	
	//lag or slowdown from mechanical/friction forces
	float ForwardSpeed = FVector::DotProduct(WheelForward, TempVel);

	FVector TempVec = -WheelForward * ForwardSpeed * EngineBrake;
		
	EnginePower = GetPowerToWheels(DeltaTime, BodyInstance);
		
	//final power output 
	//multiply grip by spring length, compressed = more grip ...
	float GripMultiplier = FMath::Min(TraceLength / SpringLengthArray[Index], MaxLatGrip);

	if (bBraking) {
		CurrentPower = Throttle * BrakeForce*GripMultiplier;// ;
	}
	else {
		CurrentPower = Throttle * EnginePower*GripMultiplier;// ;
	}
	

	//tire smoke
	if (abs(int32(CurrentPower)) >= LongSlipThreshold*2) {
		bIsSliding[Index] = true;
	}

	float ActualPower = FMath::Clamp(CurrentPower, -LongSlipThreshold, LongSlipThreshold);
	//test wasted power
	WastedPower = FMath::Abs(CurrentPower) - FMath::Abs(ActualPower);

	//TempVec += WheelForward * ActualPower;
	TempVec += WheelForward * ActualPower;

	return TempVec;

}

//lateral grip from the tire
FVector ATegCar::AddLatGrip(float DeltaTime, FBodyInstance* BodyInstance, FVector Loc, FVector Dir, int32 Index) {

	FVector TempVel = BodyInstance->GetUnrealWorldVelocityAtPoint(Loc);
	FTransform BodyTransform = BodyInstance->GetUnrealWorldTransform();
	FVector BodyLocation = BodyTransform.GetLocation();

	FVector BodyRightVector = BodyTransform.GetUnitAxis(EAxis::Y);
	FVector BodyUpVector = BodyTransform.GetUnitAxis(EAxis::Z);

	FVector WheelRight = BodyRightVector.RotateAngleAxis(CurrentAngle[Index], BodyUpVector);

	float SideSpeed = FVector::DotProduct(WheelRight, TempVel) * Grip;
	
	//tire smoke
	if (abs(int32(SideSpeed)) >= SlipThreshold*SmokeKickIn) {
		bIsSliding[Index] = true;
	}
	
	//clamp it, we dont want infinite grip
	float ActualGrip = FMath::Clamp(SideSpeed, -SlipThreshold, SlipThreshold);

	FVector TempVec = -WheelRight * ActualGrip;

	return TempVec;
}

float ATegCar::GetPowerToWheels(float DeltaTime, FBodyInstance* BodyInstance)
{
	//get velocity
	FVector TempVel = BodyInstance->GetUnrealWorldVelocity();
	//	get forward
	FTransform BodyTransform = BodyInstance->GetUnrealWorldTransform();
	FVector BodyForwardVector = BodyTransform.GetUnitAxis(EAxis::X);
	//get forward speed
	float ForwardSpeed = FVector::DotProduct(BodyForwardVector, TempVel);

	//get wheel rpm
	WheelRPM = ((60 * FMath::Abs(ForwardSpeed)) / (Radius * 2 * PI));
	//get engine rpm
	EngineRPM = WheelRPM * Gears[CurrentGear] * FinalGearRatio;

	//limit revs, and/or change gear
	if (EngineRPM < GearDownRPM) {
			if (CurrentGear > 1 && bAutomaticGears) {
			CurrentGear--;
			}
			else {
			EngineRPM = EngineIdleRPM;
			}

		}

		if (EngineRPM > GearUpRPM) {
			if (CurrentGear < (Gears.Num() - 1) && bAutomaticGears && ForwardSpeed > 0.0f) {//Gears.Num()
			CurrentGear++;
			}
			else {
			EngineRPM = EngineMaxRPM;
			}

		}

	//get power from torque curve
	AvailablePower = TorqueCurve->GetFloatValue(EngineRPM);
	//go through the gearbox
	EnginePower = AvailablePower * Gears[CurrentGear] * FinalGearRatio * 0.7 / (Radius*0.1);
	//newton/unreal conversion
	EnginePower *= 100;

	if (EngineRPM > RedLineRPM) {
		EnginePower = 0;
	}
	
	return EnginePower;
}

/** Returns Mesh subobject **/
USkeletalMeshComponent* ATegCar::GetMesh() const { return Mesh; }


