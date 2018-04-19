#include "VehicleMovementComponent.h"
#include "KartVehicle.h"

//Getters and setters
FVector UVehicleMovementComponent::GetCurrentVelocity(){return m_CurrentVelocity;}
void UVehicleMovementComponent::SetCurrentVelocity(FVector newVelocity) {m_CurrentVelocity = newVelocity;}
FKartVehicleMoveStruct UVehicleMovementComponent::GetLastMove() { return m_LastMove; }

/////////////////////////////////////////////////////////////////////////////////////////////////////
UVehicleMovementComponent::UVehicleMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	m_CachedOwner = Cast<AKartVehicle>(GetOwner());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Simulate when Server or when has autority over the current pawn
	if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		m_LastMove = CreateNewMove(DeltaTime);
		SimulateMove(m_LastMove);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
FKartVehicleMoveStruct UVehicleMovementComponent::CreateNewMove(float DeltaTime)
{
	FKartVehicleMoveStruct currentMove;
	currentMove.m_CurrentDeltaTime = DeltaTime;
	currentMove.m_SteeringThrow = m_CachedOwner->GetCurrentSteeringThrow();
	currentMove.m_Throttle = m_CachedOwner->GetCurrentThrottle();
	currentMove.m_CreationTime = GetWorld()->TimeSeconds;

	return currentMove;
}

///////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::SimulateMove(const FKartVehicleMoveStruct& moveToSimulate)
{
	UpdateLocation(moveToSimulate.m_CurrentDeltaTime, moveToSimulate.m_Throttle);
	UpdateRotation(moveToSimulate.m_CurrentDeltaTime, moveToSimulate.m_SteeringThrow);
}

////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::UpdateLocation(float DeltaTime, float throttle)
{
	FVector forceToAdd = CalculateForceToApply(throttle);
	FVector translationToApply = CalculateTranslationToApply(forceToAdd, DeltaTime);
	
	//Apply Translation
	bool shouldSweep = true;
	FHitResult outCollisionDuringTranslation;
	GetOwner()->AddActorWorldOffset(translationToApply, shouldSweep, &outCollisionDuringTranslation);

	//Clear velocity when hits obstacles
	if (outCollisionDuringTranslation.IsValidBlockingHit())
	{
		m_CurrentVelocity = FVector::ZeroVector;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void UVehicleMovementComponent::UpdateRotation(float DeltaTime, float steeringThrow)
{
	float deltaSpeed = FVector::DotProduct(GetOwner()->GetActorForwardVector(), m_CurrentVelocity) * DeltaTime;
	float rotationAngle = deltaSpeed / m_MinTurningRadius * steeringThrow;
	FQuat AppliedRotation(GetOwner()->GetActorUpVector(), rotationAngle);
	m_CurrentVelocity = AppliedRotation.RotateVector(m_CurrentVelocity); //need to rotate the velocity also
	GetOwner()->AddActorWorldRotation(AppliedRotation);
}

///////////////////////////////////////////////////////////////////////////////////////////////
FVector UVehicleMovementComponent::CalculateForceToApply(float throttle)
{
	FVector forceDirection = GetOwner()->GetActorForwardVector();
	float forceMagnitude = m_MaxDrivingForce * throttle;
	FVector forceToAdd = forceDirection * forceMagnitude;
	forceToAdd += CalculateAirResistance();
	forceToAdd += CalculateRollingResistance();
	
	return forceToAdd;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
FVector UVehicleMovementComponent::CalculateTranslationToApply(FVector forceToAdd, float DeltaTime)
{
	FVector currentAcceleration = forceToAdd / m_CurrentMass;
	m_CurrentVelocity = m_CurrentVelocity + currentAcceleration * DeltaTime;
	FVector translationToApply = m_CurrentVelocity * 100 * DeltaTime;

	return translationToApply;
}

//////////////////////////////////////////////////////////////////////////////////////////
FVector UVehicleMovementComponent::CalculateAirResistance()
{
	float currentSpeedSquare = FMath::Square(m_CurrentVelocity.Size());
	float airResistanceMagnitude = currentSpeedSquare * m_DragCoefficient;
	FVector currentAirResistance = -m_CurrentVelocity.GetSafeNormal() * airResistanceMagnitude;
	
	return currentAirResistance;
}

////////////////////////////////////////////////////////////////////////////////////////////
FVector UVehicleMovementComponent::CalculateRollingResistance()
{
	float gravity = -GetWorld()->GetGravityZ() / 100; //Division is because of unreal units
	float RollingResistanceMag = m_CurrentMass * gravity; //Normal force
	FVector RollingResistance = -m_CurrentVelocity.GetSafeNormal() * m_RollingResistanceCoef * RollingResistanceMag;

	return RollingResistance;
}



