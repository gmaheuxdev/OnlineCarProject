#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "KartVehicle.generated.h"

UCLASS()
class ONLINERACINGPROJECT_API AKartVehicle : public APawn
{
	GENERATED_BODY()

//Member variables
private:
	FVector m_CurrentVelocity;

	UPROPERTY(EditAnywhere)
	float m_CurrentMass = 1000; //KG
	UPROPERTY(Replicated)
	float m_CurrentThrottle;
	UPROPERTY(Replicated)
	float m_SteeringThrow;
	UPROPERTY(EditAnywhere)
	float m_DragCoefficient = 16;
	UPROPERTY(EditAnywhere)
	float m_RollingResistanceCoef = 0.1;
	UPROPERTY(EditAnywhere)
	float m_MaxDrivingForce = 10000; //Force at max throttle(Newtons)
	UPROPERTY(EditAnywhere)
	float m_MinTurningRadius = 10;

	//Replication
	UPROPERTY(ReplicatedUsing  = OnReplicate_ReplicatedTransform)
	FTransform m_ReplicatedTransform;

//Member methods
public:
	AKartVehicle();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	void UpdateRotation(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float value);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float value);
	void MoveForward(float value);
	void MoveRight(float value);

	//Callbacks for replication events
	UFUNCTION()
	void OnReplicate_ReplicatedTransform();

private:
	void UpdateLocation(float DeltaTime);
	FVector GetAirResistance();
	FVector GetRollingResistance();

};
