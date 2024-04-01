// Copyright 2024 jeonghun

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "LyraLocomotionTypes.generated.h"

UENUM()
enum class ERootYawOffsetMode : uint8
{
	BlendOut,
	Hold,
	Accumulate
};

UENUM()
enum class ECardinalDirection : uint8
{
	Forward,
	Backward,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct FCardinalDirections
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Forward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Backward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Left;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Right;
};
