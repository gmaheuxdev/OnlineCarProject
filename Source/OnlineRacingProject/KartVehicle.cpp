#include "KartVehicle.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "UnrealNetwork.h"

AKartVehicle::AKartVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

//////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::BeginPlay()
{
	Super::BeginPlay();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AKartVehicle, m_ReplicatedTransform); //Register variable to replication
	DOREPLIFETIME(AKartVehicle, m_CurrentVelocity);
	DOREPLIFETIME(AKartVehicle, m_SteeringThrow);
	DOREPLIFETIME(AKartVehicle, m_CurrentThrottle);
	//You need to replicate all the information the replicated transform need to have a smooth update every frame
	//If not, you will need to wait a net update wich can take a longer time and we wans to keep it less crowded to reduce lag
	// and strain to the server connection
	//When replicatedLocation is set on the server, the client's version of the variable will be
    //Set to the server's
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

	if (HasAuthority()) // Only the server replicates
	{
		m_ReplicatedTransform = GetTransform();
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

	if (HasAuthority())
	{ 
		m_ReplicatedTransform = GetTransform();
	}
	
	//Old rotation kept for training purposes
	//float rotationAngle = m_MaxRotationPerSecond * DeltaTime * m_SteeringThrow;
	//FQuat rotationDelta(GetActorUpVector(), FMath::DegreesToRadians(rotationAngle));
	//m_CurrentVelocity = rotationDelta.RotateVector(m_CurrentVelocity);
	//AddActorWorldRotation(rotationDelta);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::OnReplicate_ReplicatedTransform()
{
	//Will be called only when an update of the replicated value is needed instead of every god damn frame
	SetActorTransform(m_ReplicatedTransform);
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
	Server_MoveForward(value);
}

//////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::MoveRight(float value)
{
	m_SteeringThrow = value;
	Server_MoveRight(value);
}

///////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::Server_MoveForward_Implementation(float value)
{
	m_CurrentThrottle = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool AKartVehicle::Server_MoveForward_Validate(float value)
{
	//Cheat protection: Throttle always has to be between -1 and 1
	if (value >= -1 && value <= 1)
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::Server_MoveRight_Implementation(float value)
{
	m_SteeringThrow = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool AKartVehicle::Server_MoveRight_Validate(float value)
{
	return true;
}

