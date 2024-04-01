// Copyright 2024 jeonghun


#include "LLPlayerController.h"
#include "InputMappingContext.h"

void ALLPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	MappingContext = NewObject<UInputMappingContext>(this);

	MoveAction = NewObject<UInputAction>(this);
	MoveAction->ValueType = EInputActionValueType::Axis3D;

	MappingContext->MapKey(MoveAction, EKeys::W).Modifiers.Add(NewObject<UInputModifierSwizzleAxis>());
	MappingContext->MapKey(MoveAction, EKeys::S).Modifiers.Append({NewObject<UInputModifierNegate>(this), NewObject<UInputModifierSwizzleAxis>()});
	MappingContext->MapKey(MoveAction, EKeys::A).Modifiers.Add(NewObject<UInputModifierNegate>(this));
	MappingContext->MapKey(MoveAction, EKeys::D);

	LookAction = NewObject<UInputAction>(this);
	LookAction->ValueType = EInputActionValueType::Axis2D;
	MappingContext->MapKey(LookAction, EKeys::Mouse2D);

	JumpAction = NewObject<UInputAction>(this);
	JumpAction->ValueType = EInputActionValueType::Boolean;
	MappingContext->MapKey(JumpAction, EKeys::SpaceBar).Triggers.Add(NewObject<UInputTriggerPressed>(this));
}
