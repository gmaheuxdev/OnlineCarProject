#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleMovementComponent.h"
#include "UnrealNetwork.h"
#include "ReplicationComponent.generated.h"

////////////////////////////////////////////////////////////
USTRUCT()
struct FKartVehicleStateStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FKartVehicleMoveStruct m_LastMove;
	UPROPERTY()
	FVector m_currentVelocity;
	UPROPERTY()
	FTransform m_CurrentTransform;

};
//////////////////////////////////////////////////////////////

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ONLINERACINGPROJECT_API UReplicationComponent : public UActorComponent
{
	GENERATED_BODY()


//Member variables
private:
UVehicleMovementComponent* m_CachedMovementComponent;
TArray<FKartVehicleMoveStruct> m_MoveQueueArray; //Moves waiting to be analaysed by server


//Member methods
public:	
	
	UReplicationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FKartVehicleMoveStruct moveToSend);
	
protected:
	
	virtual void BeginPlay() override;

private:

	//Replication
	UPROPERTY(ReplicatedUsing = OnReplicate_CurrentServerState)
	FKartVehicleStateStruct m_currentServerState;
	UFUNCTION()
	void OnReplicate_CurrentServerState();
	void ClearAcknowledgedMoves(FKartVehicleMoveStruct lastMove);

};
