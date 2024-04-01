// Copyright 2024 jeonghun


#include "LLGameMode.h"
#include "LLCharacter.h"
#include "LLPlayerController.h"

ALLGameMode::ALLGameMode()
{
	DefaultPawnClass = ALLCharacter::StaticClass();
	PlayerControllerClass = ALLPlayerController::StaticClass();
}
