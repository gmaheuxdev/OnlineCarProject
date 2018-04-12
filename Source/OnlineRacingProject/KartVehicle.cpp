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
	DOREPLIFETIME(AKartVehicle, m_currentServerState);
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

	if (GetRemoteRole() == ENetRole::ROLE_SimulatedProxy && Role == ROLE_Authority )
	{
		FKartVehicleMoveStruct currentClientMove;
		currentClientMove.m_CurrentDeltaTime = DeltaTime;
		currentClientMove.m_SteeringThrow = m_CurrentSteeringThrow;
		currentClientMove.m_Throttle = m_CurrentThrottle;
		currentClientMove.m_CreationTime = GetWorld()->TimeSeconds;
		SimulateMove(currentClientMove);
		Server_SendMove(currentClientMove);
	}

	if (Role == ROLE_AutonomousProxy)
	{
		FKartVehicleMoveStruct currentClientMove;
		currentClientMove.m_CurrentDeltaTime = DeltaTime;
		currentClientMove.m_SteeringThrow = m_CurrentSteeringThrow;
		currentClientMove.m_Throttle = m_CurrentThrottle;
		currentClientMove.m_CreationTime = GetWorld()->TimeSeconds;
		SimulateMove(currentClientMove);
		m_MoveQueueArray.Add(currentClientMove);
		Server_SendMove(currentClientMove);
	}

	if (Role == ROLE_SimulatedProxy) //Role on the current computer
	{
		SimulateMove(m_currentServerState.m_LastMove);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::UpdateLocation(float DeltaTime, float throttle)
{
	FVector forceDirection = GetActorForwardVector();
	float forceMagnitude = m_MaxDrivingForce * throttle;
	FVector forceToAdd = forceDirection * forceMagnitude;
	forceToAdd += GetAirResistance();
	forceToAdd += GetRollingResistance();
	FVector currentAcceleration = forceToAdd / m_CurrentMass;
	m_CurrentVelocity = m_CurrentVelocity + currentAcceleration * DeltaTime;
	FVector translationToApply = m_CurrentVelocity * 100 * DeltaTime;

	bool shouldSweep = true;
	FHitResult outCollisionDuringTranslation;
	AddActorWorldOffset(translationToApply, shouldSweep, &outCollisionDuringTranslation);

	if (outCollisionDuringTranslation.IsValidBlockingHit()) //Clear velocity when hitting obstacle
	{
		m_CurrentVelocity = FVector::ZeroVector;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::UpdateRotation(float DeltaTime, float steeringThrow)
{
	float deltaSpeed = FVector::DotProduct(GetActorForwardVector(), m_CurrentVelocity) * DeltaTime;
	float testval = m_CurrentVelocity.Size() * DeltaTime; // same as dotproduct when positive
	float rotationAngle = deltaSpeed / m_MinTurningRadius * steeringThrow;
	FQuat AppliedRotation(GetActorUpVector(), rotationAngle);
	m_CurrentVelocity = AppliedRotation.RotateVector(m_CurrentVelocity);
	AddActorWorldRotation(AppliedRotation);

	if (HasAuthority())
	{ 
		m_currentServerState.m_CurrentTransform = GetTransform();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::OnReplicate_CurrentServerState()
{
	//Will be called only when an update of the replicated value is needed instead of every god damn frame
	SetActorTransform(m_currentServerState.m_CurrentTransform);
	m_CurrentVelocity = m_currentServerState.m_currentVelocity;
	ClearAcknowledgedMoves(m_currentServerState.m_LastMove);

	for (const FKartVehicleMoveStruct& move : m_MoveQueueArray)
	{
		SimulateMove(move);
	}

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
	m_CurrentSteeringThrow = value;
}

///////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::Server_SendMove_Implementation(FKartVehicleMoveStruct moveToSend) //Called when arrived on server
{
	SimulateMove(moveToSend);
	
	m_currentServerState.m_LastMove = moveToSend;
	m_currentServerState.m_CurrentTransform = GetTransform();
	m_currentServerState.m_currentVelocity = m_CurrentVelocity;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool AKartVehicle::Server_SendMove_Validate(FKartVehicleMoveStruct moveToSend)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::SimulateMove(const FKartVehicleMoveStruct& moveToSimulate)
{
	UpdateLocation(moveToSimulate.m_CurrentDeltaTime,moveToSimulate.m_Throttle);
	UpdateRotation(moveToSimulate.m_CurrentDeltaTime,moveToSimulate.m_SteeringThrow);
}

////////////////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::ClearAcknowledgedMoves(FKartVehicleMoveStruct lastMove)
{
	TArray<FKartVehicleMoveStruct> newMoves;

	for (const FKartVehicleMoveStruct& move : m_MoveQueueArray)
	{
		if (move.m_CreationTime > lastMove.m_CreationTime)
		{
			newMoves.Add(move);
		}
	}

	m_MoveQueueArray = newMoves;
}
