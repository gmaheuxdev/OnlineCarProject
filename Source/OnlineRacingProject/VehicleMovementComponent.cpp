#include "VehicleMovementComponent.h"

//Getters and setters
FVector UVehicleMovementComponent::GetCurrentVelocity(){return m_CurrentVelocity;}
void UVehicleMovementComponent::SetCurrentVelocity(FVector newVelocity) {m_CurrentVelocity = newVelocity;}

/////////////////////////////////////////////////////////////////////////////////////////////////////
UVehicleMovementComponent::UVehicleMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


//////////////////////////////////////////////////////////////////////////////////////////
FVector UVehicleMovementComponent::GetAirResistance()
{
	float currentSpeedSquare = FMath::Square(m_CurrentVelocity.Size());
	float airResistanceMagnitude = currentSpeedSquare * m_DragCoefficient;
	FVector currentAirResistance = -m_CurrentVelocity.GetSafeNormal() * airResistanceMagnitude;
	return currentAirResistance;
}

////////////////////////////////////////////////////////////////////////////////////////////
FVector UVehicleMovementComponent::GetRollingResistance()
{
	float gravity = -GetWorld()->GetGravityZ() / 100; //Division is because of unreal units
	float RollingResistanceMag = m_CurrentMass * gravity; //Normal force
	FVector RollingResistance = -m_CurrentVelocity.GetSafeNormal() * m_RollingResistanceCoef * RollingResistanceMag;

	return RollingResistance;
}

////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::UpdateLocation(float DeltaTime, float throttle)
{
	FVector forceDirection = GetOwner()->GetActorForwardVector();
	float forceMagnitude = m_MaxDrivingForce * throttle;
	FVector forceToAdd = forceDirection * forceMagnitude;
	forceToAdd += GetAirResistance();
	forceToAdd += GetRollingResistance();
	FVector currentAcceleration = forceToAdd / m_CurrentMass;
	m_CurrentVelocity = m_CurrentVelocity + currentAcceleration * DeltaTime;
	FVector translationToApply = m_CurrentVelocity * 100 * DeltaTime;

	bool shouldSweep = true;
	FHitResult outCollisionDuringTranslation;
	GetOwner()->AddActorWorldOffset(translationToApply, shouldSweep, &outCollisionDuringTranslation);

	if (outCollisionDuringTranslation.IsValidBlockingHit()) //Clear velocity when hitting obstacle
	{
		m_CurrentVelocity = FVector::ZeroVector;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::UpdateRotation(float DeltaTime, float steeringThrow)
{
	float deltaSpeed = FVector::DotProduct(GetOwner()->GetActorForwardVector(), m_CurrentVelocity) * DeltaTime;
	float testval = m_CurrentVelocity.Size() * DeltaTime; // same as dotproduct when positive
	float rotationAngle = deltaSpeed / m_MinTurningRadius * steeringThrow;
	FQuat AppliedRotation(GetOwner()->GetActorUpVector(), rotationAngle);
	m_CurrentVelocity = AppliedRotation.RotateVector(m_CurrentVelocity);
	GetOwner()->AddActorWorldRotation(AppliedRotation);
}

///////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::SimulateMove(const FKartVehicleMoveStruct& moveToSimulate)
{
	UpdateLocation(moveToSimulate.m_CurrentDeltaTime, moveToSimulate.m_Throttle);
	UpdateRotation(moveToSimulate.m_CurrentDeltaTime, moveToSimulate.m_SteeringThrow);
}