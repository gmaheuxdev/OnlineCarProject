#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehicleMovementComponent.h"
#include "ReplicationComponent.h"
#include "KartVehicle.generated.h"


UCLASS()
class ONLINERACINGPROJECT_API AKartVehicle : public APawn
{
	GENERATED_BODY()

public:
//Getters and setters
void SetCurrentThrottle(float newThrottle);
float GetCurrentThrottle();
void SetCurrentSteeringThrow(float newSteeringThrow);
float GetCurrentSteeringThrow();


//Member variables
private:
	
	
	UPROPERTY()
	FKartVehicleMoveStruct m_CurrentMove;
	float m_CurrentThrottle;
	float m_CurrentSteeringThrow;

	class  UVehicleMovementComponent* m_CachedMovementComponent;
	class  UReplicationComponent* m_CachedReplicationComponent;


//Member methods
public:
	AKartVehicle();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void MoveForward(float value);
	void MoveRight(float value);
};
