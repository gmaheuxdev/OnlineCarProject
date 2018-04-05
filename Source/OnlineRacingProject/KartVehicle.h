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
	float m_CurrentThrottle;
	float m_SteeringThrow;
	UPROPERTY(EditAnywhere)
	float m_DragCoefficient = 16;
	UPROPERTY(EditAnywhere)
	float m_RollingResistanceCoef = 0.1;
	UPROPERTY(EditAnywhere)
	float m_MaxDrivingForce = 10000; //Force at max throttle(Newtons)
	UPROPERTY(EditAnywhere)
	float m_MinTurningRadius = 10;

//Member methods
public:
	AKartVehicle();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	void UpdateRotation(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void MoveForward(float value);
	void MoveRight(float value);

private:
	void UpdateLocation(float DeltaTime);
	FVector GetAirResistance();
	FVector GetRollingResistance();

};
