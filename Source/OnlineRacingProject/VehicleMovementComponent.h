#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleMovementComponent.generated.h"

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ONLINERACINGPROJECT_API UVehicleMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
//Getters and setters
FVector GetCurrentVelocity();
void SetCurrentVelocity(FVector newVelocity);

//Member variables
private:
	
	FVector m_CurrentVelocity;

	UPROPERTY(EditAnywhere)
	float m_CurrentMass = 1000; //KG
	
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

	UVehicleMovementComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SimulateMove(const FKartVehicleMoveStruct& moveToSimulate);

protected:

	virtual void BeginPlay() override;


private:

	FVector GetAirResistance();
	FVector GetRollingResistance();
	void UpdateLocation(float DeltaTime, float throttle);
	void UpdateRotation(float DeltaTime, float steeringThrow);
};
