// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OnlineRacingProjectGameMode.h"
#include "OnlineRacingProjectPawn.h"
#include "OnlineRacingProjectHud.h"

AOnlineRacingProjectGameMode::AOnlineRacingProjectGameMode()
{
	DefaultPawnClass = AOnlineRacingProjectPawn::StaticClass();
	HUDClass = AOnlineRacingProjectHud::StaticClass();
}
