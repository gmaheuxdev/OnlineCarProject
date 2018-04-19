#include "ReplicationComponent.h"
#include "VehicleMovementComponent.h"
#include "KartVehicle.h"
#include "UnrealNetwork.h"

UReplicationComponent::UReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::BeginPlay()
{
	Super::BeginPlay();
	m_CachedMovementComponent = GetOwner()->FindComponentByClass<UVehicleMovementComponent>();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (m_CachedMovementComponent != nullptr)
	{
		FKartVehicleMoveStruct lastMove = m_CachedMovementComponent->GetLastMove();
		UpdateReplication(lastMove);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::UpdateReplication(FKartVehicleMoveStruct lastMove)
{
	if (GetOwnerRole() == ROLE_AutonomousProxy) // I am in control of this pawn
	{
		m_MoveQueueArray.Add(lastMove);
		Server_SendMove(lastMove);
	}

	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy) // I am the server
	{
		UpdateServerState(lastMove);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy) //Simulate last move sent by server
	{
		m_CachedMovementComponent->SimulateMove(m_currentServerState.m_LastMove);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::UpdateServerState(FKartVehicleMoveStruct& move)
{
	m_currentServerState.m_LastMove = move;
	m_currentServerState.m_CurrentTransform = GetOwner()->GetTransform();
	m_currentServerState.m_currentVelocity = m_CachedMovementComponent->GetCurrentVelocity();
}

//////////////////////////////////////////////////////////////////////////////////////////
bool UReplicationComponent::Server_SendMove_Validate(FKartVehicleMoveStruct moveToCheck)
{
	if (!IsMoveValid(moveToCheck)) {return false;}
	if (!IsDeltaTimeValid(moveToCheck.m_CurrentDeltaTime)) {return false;}
		
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
bool UReplicationComponent::IsMoveValid(FKartVehicleMoveStruct moveToValidate)
{
	//Validate Steering and throttle
	if (moveToValidate.m_SteeringThrow > 1 || moveToValidate.m_Throttle > 1)
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
bool UReplicationComponent::IsDeltaTimeValid(float timeToCheck)
{
	//if the client's time is bigger thant the server's it means that it goes too fast on client
	float potentialTime = m_SimulatedDeltaTime + timeToCheck;
	if (potentialTime < GetWorld()->TimeSeconds) { return true; }

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UReplicationComponent, m_currentServerState);
}

///////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::Server_SendMove_Implementation(FKartVehicleMoveStruct moveSentToServer) //Called when arrived on server
{
	if (m_CachedMovementComponent != nullptr)
	{
		m_SimulatedDeltaTime += moveSentToServer.m_CurrentDeltaTime;
		m_CachedMovementComponent->SimulateMove(moveSentToServer);
		UpdateServerState(moveSentToServer);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void UReplicationComponent::OnReplicate_CurrentServerState()
{
	if (m_CachedMovementComponent != nullptr)
	{
		//Will be called only when an update of the replicated value is needed
		GetOwner()->SetActorTransform(m_currentServerState.m_CurrentTransform);
		m_CachedMovementComponent->SetCurrentVelocity(m_currentServerState.m_currentVelocity);
		ClearAcknowledgedMoves(m_currentServerState.m_LastMove);

		for (const FKartVehicleMoveStruct& move : m_MoveQueueArray)
		{
			m_CachedMovementComponent->SimulateMove(move);
		}
	}
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
