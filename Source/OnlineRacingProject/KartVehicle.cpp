#include "KartVehicle.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"

AKartVehicle::AKartVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
}

//////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::BeginPlay()
{
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateLocation(DeltaTime);
	UpdateRotation(DeltaTime);
}

////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::UpdateLocation(float DeltaTime)
{
	FVector forceDirection = GetActorForwardVector();
	float forceMagnitude = m_MaxDrivingForce * m_CurrentThrottle;
	FVector forceToAdd = forceDirection * forceMagnitude;
	forceToAdd += GetAirResistance();
	forceToAdd += GetRollingResistance();
	FVector currentAcceleration = forceToAdd / m_CurrentMass;
	m_CurrentVelocity = m_CurrentVelocity + currentAcceleration * DeltaTime;
	FVector translationToApply = m_CurrentVelocity * 100 * DeltaTime;

	bool shouldSweep = true;
	FHitResult outCollisionDuringTranslation;
	AddActorWorldOffset(translationToApply, shouldSweep, &outCollisionDuringTranslation);

	if (outCollisionDuringTranslation.IsValidBlockingHit())
	{
		m_CurrentVelocity = FVector::ZeroVector;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::UpdateRotation(float DeltaTime)
{
	float deltaSpeed = FVector::DotProduct(GetActorForwardVector(), m_CurrentVelocity) * DeltaTime;
	float testval = m_CurrentVelocity.Size() * DeltaTime; // same as dotproduct when positive
	float rotationAngle = deltaSpeed / m_MinTurningRadius * m_SteeringThrow;
	FQuat AppliedRotation(GetActorUpVector(), rotationAngle);
	m_CurrentVelocity = AppliedRotation.RotateVector(m_CurrentVelocity);
	AddActorWorldRotation(AppliedRotation);

	//Old rotation kept for training purposes
	//float rotationAngle = m_MaxRotationPerSecond * DeltaTime * m_SteeringThrow;
	//FQuat rotationDelta(GetActorUpVector(), FMath::DegreesToRadians(rotationAngle));
	//m_CurrentVelocity = rotationDelta.RotateVector(m_CurrentVelocity);
	//AddActorWorldRotation(rotationDelta);
}



//////////////////////////////////////////////////////////////////////////////////////////
FVector AKartVehicle::GetAirResistance()
{
	float currentSpeedSquare = FMath::Square(m_CurrentVelocity.Size());
	float airResistanceMagnitude = currentSpeedSquare * m_DragCoefficient;
	FVector currentAirResistance = - m_CurrentVelocity.GetSafeNormal() * airResistanceMagnitude;
	return currentAirResistance;
}

////////////////////////////////////////////////////////////////////////////////////////////
FVector AKartVehicle::GetRollingResistance()
{
	float gravity = -GetWorld()->GetGravityZ() / 100; //Division is because of unreal units
	float RollingResistanceMag = m_CurrentMass * gravity; //Normal force
	FVector RollingResistance = -m_CurrentVelocity.GetSafeNormal() * m_RollingResistanceCoef * RollingResistanceMag;
	
	return RollingResistance;
}

////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AKartVehicle::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AKartVehicle::MoveRight);
}

///////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::MoveForward(float value)
{
	m_CurrentThrottle = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::MoveRight(float value)
{
	m_SteeringThrow = value;
}

