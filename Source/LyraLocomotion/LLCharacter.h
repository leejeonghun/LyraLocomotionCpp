// Copyright 2024 jeonghun

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LLCharacter.generated.h"

UCLASS()
class LYRALOCOMOTION_API ALLCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "ture"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

public:
	ALLCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void Move(const struct FInputActionValue& Value);
	virtual void Look(const struct FInputActionValue& Value);
};
