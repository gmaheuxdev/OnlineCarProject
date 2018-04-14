#include "ReplicationComponent.h"
#include "VehicleMovementComponent.h"
#include "KartVehicle.h"
#include "UnrealNetwork.h"

UReplicationComponent::UReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReplicationComponent::BeginPlay()
{
	Super::BeginPlay();
	m_CachedMovementComponent = GetOwner()->FindComponentByClass<UVehicleMovementComponent>();
}

void UReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->GetRemoteRole() == ENetRole::ROLE_SimulatedProxy && GetOwner()->Role == ROLE_Authority)
	{
		FKartVehicleMoveStruct currentClientMove;
		currentClientMove.m_CurrentDeltaTime = DeltaTime;
		currentClientMove.m_SteeringThrow = Cast<AKartVehicle>(GetOwner())->GetCurrentSteeringThrow();
		currentClientMove.m_Throttle = Cast<AKartVehicle>(GetOwner())->GetCurrentThrottle();
		currentClientMove.m_CreationTime = GetWorld()->TimeSeconds;

		if (m_CachedMovementComponent != nullptr)
		{
			m_CachedMovementComponent->SimulateMove(currentClientMove);
			Server_SendMove(currentClientMove);
		}
	}

	if (GetOwner()->Role == ROLE_AutonomousProxy)
	{
		FKartVehicleMoveStruct currentClientMove;
		currentClientMove.m_CurrentDeltaTime = DeltaTime;
		currentClientMove.m_SteeringThrow = Cast<AKartVehicle>(GetOwner())->GetCurrentSteeringThrow();
		currentClientMove.m_Throttle = Cast<AKartVehicle>(GetOwner())->GetCurrentThrottle();
		currentClientMove.m_CreationTime = GetWorld()->TimeSeconds;

		if (m_CachedMovementComponent != nullptr)
		{
			m_CachedMovementComponent->SimulateMove(currentClientMove);
		}

		m_MoveQueueArray.Add(currentClientMove);
		Server_SendMove(currentClientMove);
	}

	if (GetOwner()->Role == ROLE_SimulatedProxy) //Role on the current computer
	{
		if (m_CachedMovementComponent != nullptr)
		{
			m_CachedMovementComponent->SimulateMove(m_currentServerState.m_LastMove);
		}
	}

}

/////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::OnReplicate_CurrentServerState()
{
	if (m_CachedMovementComponent != nullptr)
	{
		//Will be called only when an update of the replicated value is needed instead of every god damn frame
		GetOwner()->SetActorTransform(m_currentServerState.m_CurrentTransform);
		m_CachedMovementComponent->SetCurrentVelocity(m_currentServerState.m_currentVelocity);
		ClearAcknowledgedMoves(m_currentServerState.m_LastMove);

		for (const FKartVehicleMoveStruct& move : m_MoveQueueArray)
		{
			m_CachedMovementComponent->SimulateMove(move);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::Server_SendMove_Implementation(FKartVehicleMoveStruct moveToSend) //Called when arrived on server
{
	if (m_CachedMovementComponent != nullptr)
	{
		m_CachedMovementComponent->SimulateMove(moveToSend);

		m_currentServerState.m_LastMove = moveToSend;
		m_currentServerState.m_CurrentTransform = GetOwner()->GetTransform();
		m_currentServerState.m_currentVelocity = m_CachedMovementComponent->GetCurrentVelocity();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
bool UReplicationComponent::Server_SendMove_Validate(FKartVehicleMoveStruct moveToSend)
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::ClearAcknowledgedMoves(FKartVehicleMoveStruct lastMove)
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

/////////////////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UReplicationComponent, m_currentServerState);
	//You need to replicate all the information the replicated transform need to have a smooth update every frame
	//If not, you will need to wait a net update wich can take a longer time and we wans to keep it less crowded to reduce lag
	// and strain to the server connection
	//When replicatedLocation is set on the server, the client's version of the variable will be
	//Set to the server's
}
