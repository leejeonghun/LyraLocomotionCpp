// Copyright 2024 jeonghun

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LLPlayerController.generated.h"

UCLASS()
class LYRALOCOMOTION_API ALLPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	UPROPERTY()
	TObjectPtr<class UInputMappingContext> MappingContext;

	UPROPERTY()
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<class UInputAction> JumpAction;
};
