#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "KartVehicle.generated.h"

USTRUCT()
struct FKartVehicleMoveStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float m_Throttle;
	UPROPERTY()
	float m_SteeringThrow;
	UPROPERTY()
	float m_CurrentDeltaTime;
	UPROPERTY()
	float m_CreationTime;
};

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

UCLASS()
class ONLINERACINGPROJECT_API AKartVehicle : public APawn
{
	GENERATED_BODY()

//Member variables
private:

	FVector m_CurrentVelocity;

	UPROPERTY(EditAnywhere)
	float m_CurrentMass = 1000; //KG
	float m_CurrentThrottle;
	float m_CurrentSteeringThrow;
	UPROPERTY(EditAnywhere)
	float m_DragCoefficient = 16;
	UPROPERTY(EditAnywhere)
	float m_RollingResistanceCoef = 0.1;
	UPROPERTY(EditAnywhere)
	float m_MaxDrivingForce = 10000; //Force at max throttle(Newtons)
	UPROPERTY(EditAnywhere)
	float m_MinTurningRadius = 10;

	//Replication
	
	UPROPERTY(ReplicatedUsing = OnReplicate_CurrentServerState)
	FKartVehicleStateStruct m_currentServerState;
	UPROPERTY()
	FKartVehicleMoveStruct m_CurrentMove;
	TArray<FKartVehicleMoveStruct> m_MoveQueueArray; //Moves waiting to be analaysed by server
	

//Member methods
public:
	AKartVehicle();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FKartVehicleMoveStruct moveToSend);
	void MoveForward(float value);
	void MoveRight(float value);

	//Callbacks for replication events
	UFUNCTION()
	void OnReplicate_CurrentServerState();

private:
	void UpdateLocation(float DeltaTime, float throttle);
	void UpdateRotation(float DeltaTime, float steeringThrow);
	FVector GetAirResistance();
	FVector GetRollingResistance();
	void SimulateMove(const FKartVehicleMoveStruct& moveToSimulate);
	void ClearAcknowledgedMoves(FKartVehicleMoveStruct lastMove);

};
