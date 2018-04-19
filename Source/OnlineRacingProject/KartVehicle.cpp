#include "KartVehicle.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "VehicleMovementComponent.h"
#include "ReplicationComponent.h"

//Getters and Setters
void AKartVehicle::SetCurrentThrottle(float newThrottle){m_CurrentThrottle = newThrottle;}
float AKartVehicle::GetCurrentThrottle(){return m_CurrentThrottle;}
void AKartVehicle::SetCurrentSteeringThrow(float newSteeringThrow) { m_CurrentSteeringThrow = newSteeringThrow;}
float AKartVehicle::GetCurrentSteeringThrow() {return m_CurrentSteeringThrow;}

//////////////////////////////////////////////////////////////////////////////////////
AKartVehicle::AKartVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

//////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::BeginPlay()
{
	Super::BeginPlay();

	m_CachedMovementComponent = FindComponentByClass<UVehicleMovementComponent>();
	m_CachedReplicationComponent = FindComponentByClass<UReplicationComponent>();
}

//////////////////////////////////////////////////////////////////////////////////
void AKartVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
