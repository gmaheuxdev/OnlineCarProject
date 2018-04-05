// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "OnlineRacingProjectHud.generated.h"


UCLASS(config = Game)
class AOnlineRacingProjectHud : public AHUD
{
	GENERATED_BODY()

public:
	AOnlineRacingProjectHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
