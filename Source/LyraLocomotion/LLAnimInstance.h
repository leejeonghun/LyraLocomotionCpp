// Copyright 2024 jeonghun

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "LyraLocomotionTypes.h"
#include "LLAnimInstance.generated.h"

UCLASS()
class LYRALOCOMOTION_API ULLAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	UFUNCTION(BlueprintPure, Category = "Distance Matching", meta = (BlueprintThreadSafe))
	bool ShouldDistanceMatchStop() const;

	UFUNCTION(BlueprintPure, Category = "Distance Matching", meta = (BlueprintThreadSafe))
	double GetPredictedStopDistance() const;

	UFUNCTION(BlueprintPure, Category = "Idle Breaks", meta = (BlueprintThreadSafe))
	bool CanPlayIdleBreak() const;

	UFUNCTION(BlueprintPure, Category = "Helper Functions", meta = (BlueprintThreadSafe))
	bool IsMovingPerpendicularToInitialPivot() const;

	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void UpdateIdleTurnYawState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void LandRecoveryStart(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void SetupIdleState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void UpdateIdleState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Turn In Place", meta = (BlueprintThreadSafe))
	void SetUpTurnInPlaceRotationState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Turn In Place", meta = (BlueprintThreadSafe))
	void SetUpTurnInPlaceRecoveryState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void SetUpStartState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void UpdateStartState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void UpdateStopState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void SetUpPivotState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
	UFUNCTION(BlueprintCallable, Category = "State Node Functions", meta = (BlueprintThreadSafe))
	void UpdatePivotState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void UpdateIdleAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void SetUpIdleBreakAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void SetUpStartAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void UpdateStartAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void UpdateCycleAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void SetUpStopAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void UpdateStopAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void SetUpPivotAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void UpdatePivotAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void SetUpFallLandAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
	UFUNCTION(BlueprintCallable, Category = "Anim Node Functions", meta = (BlueprintThreadSafe))
	void UpdateFallLandAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Turn In Place", meta = (BlueprintThreadSafe))
	void SetupTurnInPlaceAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Turn In Place", meta = (BlueprintThreadSafe))
	void UpdateTurnInPlaceAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Turn In Place", meta = (BlueprintThreadSafe))
	void UpdateTurnInPlaceRecoveryAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	void ProcessTurnYawCurve();
	void SetRootYawOffset(float InRootYawOffset);
	TObjectPtr<UAnimSequence> SelectTurnInPlaceAnimation(float Direction) const;
	float GetGroundDistance(TObjectPtr<ACharacter> Owner);

	void UpdateLocationData(float DeltaTime);
	void UpdateRotationData();
	void UpdateVelocityData();
	void UpdateAccelerationData();
	void UpdateRootYawOffset(float InDeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
	FName LocomotionDistanceCurveName = TEXT("Distance");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
	FName JumpDistanceCurveName = TEXT("GroundDistance");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location Data")
	FVector WorldLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location Data")
	float DisplacementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rotation Data")
	FRotator WorldRotation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rotation Data")
	float AdditiveLeanAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Data")
	FVector WorldVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Data")
	float LocalVelocityDirectionAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Data")
	float LocalVelocityDirectionAngleWithOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Data")
	ECardinalDirection LocalVelocityDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Data")
	uint8 bHasVelocity : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Data")
	FVector LocalVelocity2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LocomotionSM Data")
	ECardinalDirection StartDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LocomotionSM Data")
	float LastPivotTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pivots")
	FVector PivotStartingAcceleration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Acceleration Data")
	FVector LocalAcceleration2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Acceleration Data")
	uint8 bHasAcceleration : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle Breaks")
	float TimeUntilNextIdleBreak;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float RootYawOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float TurnInPlaceAnimTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stride Warping")
	float StrideWarpingStartAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stride Warping")
	float StrideWarpingCycleAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stride Warping")
	float StrideWarpingPivotAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump")
	float LandRecoveryAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State Data")
	float GroundDistance = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State Data")
	float TimeToJumpApex = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State Data")
	uint8 bIsRunningIntoWall : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State Data")
	uint8 bIsOnGround : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State Data")
	uint8 bIsJumping : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State Data")
	uint8 bIsFalling : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Idle")
	TObjectPtr<UAnimSequence> IdleAnimSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Idle")
	TArray<TObjectPtr<UAnimSequence>> IdleBreakAnimSequences;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Turn in Place")
	TObjectPtr<UAnimSequence> TurnInPlaceLeftAnimSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Turn in Place")
	TObjectPtr<UAnimSequence> TurnInPlaceRightAnimSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Starts")
	FCardinalDirections JogStartCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Jog")
	FCardinalDirections JogCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Stops")
	FCardinalDirections JogStopCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Pivots")
	FCardinalDirections JogPivotCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Jump")
	TObjectPtr<UAnimSequence> JumpStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Jump")
	TObjectPtr<UAnimSequence> JumpStartLoop;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Jump")
	TObjectPtr<UAnimSequence> JumpApex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Jump")
	TObjectPtr<UAnimSequence> JumpFallLand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Jump")
	TObjectPtr<UAnimSequence> JumpFallLoop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim Set - Jump")
	TObjectPtr<UAnimSequence> JumpRecoveryAdditive;

private:
	static TObjectPtr<UAnimSequence> SelectDirectionalAnimation(const FCardinalDirections &Cardinals, ECardinalDirection Direction);
	static ECardinalDirection SelectCardinalDirectionFromAngle(float Angle, float DeadZone, ECardinalDirection CurrentDirection, bool bUseCurrentDirection);
	static ECardinalDirection GetOppositeCardinalDirection(ECardinalDirection CurrentDirection);
	
	bool bIsFirstUpdate = true;
	bool bWasMovingLastUpdate = false; 
	bool bIsAnyMontagePlaying = false;
	bool bUseSeparateBrakingFriction = false;
	float GroundFriction = 0;
	float BrakingFriction = 0;
	float BrakingFrictionFactor = 0;
	float BrakingDecelerationWalking = 0;
	FVector CurrAcceleration { 0 };
	FVector LastUpdateVelocity { 0 };

	// Location Data
	float DisplacementSinceLastUpdate = 0;
	FVector PrevWorldLocation { 0 };

	// Rotation Data
	float YawDeltaSinceLastUpdate = 0.f;
	
	// Turn In Place
	float TurnYawCurveValue = 0.f;
	float TurnInPlaceRotationDirection = 0.f;
	float TurnInPlaceRecoveryDirection = 0.f;
	FRotator PrevWorldRotation;
	FVector2D RootYawOffsetAngleClamp { -120, 100 };
	FFloatSpringState RootYawOffsetSpringState;
	ERootYawOffsetMode RootYawOffsetMode;

	// Idle Breaks
	float IdleBreakDelayTime = 0.f;
	uint8 CurrentIdleBreakIndex = 0;

	// Settings
	float StrideWarpingBlendInStartOffset = 0.15f;
	float StrideWarpingBlendInDurationScaled = 0.2f;
	float CardinalDirectionDeadZone = 10.0f;
	FVector2D PlayRateClampCycle { 0.8f, 1.2f };
	FVector2D PlayRateClampStartsPivots { 0.6f, 5.0f };

	// Velocity Data
	ECardinalDirection LocalVelocityDirectionNoOffset;

	// Acceleration Data
	FVector PivotDirection2D { 0 };

	// Locomotion SM Data
	ECardinalDirection PivotInitialDirection;
	ECardinalDirection CardinalDirectionFromAcceleration;
	
	// Character State Data


	// Pivots
	float TimeAtPivotStop = 0;

	// Ground Distance
	uint64 LastUpdateFrame = 0;
	float LastGroundDistance = 0;

	// Jump
	float TimeFalling = 0;
};
