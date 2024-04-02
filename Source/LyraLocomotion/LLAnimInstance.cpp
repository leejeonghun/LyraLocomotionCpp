// Copyright 2024 jeonghun


#include "LLAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimNodeReference.h"
#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "SequenceEvaluatorLibrary.h"
#include "SequencePlayerLibrary.h"
#include "AnimationStateMachineLibrary.h"
#include "AnimExecutionContextLibrary.h"
#include "KismetAnimationLibrary.h"

void ULLAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (const TObjectPtr<ACharacter> Owner = Cast<ACharacter>(GetOwningActor()))
	{
		CurrAcceleration = Owner->GetCharacterMovement()->GetCurrentAcceleration();
		PrevWorldLocation = WorldLocation;
		WorldLocation = Owner->GetActorLocation();
		PrevWorldRotation = WorldRotation;
		WorldRotation = Owner->GetActorRotation();
		WorldVelocity = Owner->GetVelocity();
		LastUpdateVelocity = Owner->GetCharacterMovement()->GetLastUpdateVelocity();
		bUseSeparateBrakingFriction = Owner->GetCharacterMovement()->bUseSeparateBrakingFriction;
		BrakingFrictionFactor = Owner->GetCharacterMovement()->BrakingFrictionFactor;
		GroundFriction = Owner->GetCharacterMovement()->GroundFriction;
		BrakingFriction = Owner->GetCharacterMovement()->BrakingFriction;
		BrakingDecelerationWalking = Owner->GetCharacterMovement()->BrakingDecelerationWalking;
		bIsOnGround = Owner->GetCharacterMovement()->IsMovingOnGround();
		bIsJumping = Owner->GetCharacterMovement()->MovementMode == MOVE_Falling && WorldVelocity.Z > 0;
		bIsFalling = Owner->GetCharacterMovement()->MovementMode == MOVE_Falling && WorldVelocity.Z <= 0;
		TimeToJumpApex = bIsJumping ? -WorldVelocity.Z / Owner->GetCharacterMovement()->GetGravityZ() : 0;
		TimeFalling = bIsFalling ? TimeFalling + DeltaSeconds : bIsJumping ? 0 : TimeFalling;
		bIsAnyMontagePlaying = IsAnyMontagePlaying();
		GroundDistance = GetGroundDistance(Owner);
	}
}

void ULLAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateLocationData(DeltaSeconds);
	UpdateRotationData();
	UpdateVelocityData();
	UpdateAccelerationData();
	UpdateRootYawOffset(DeltaSeconds);

	bIsFirstUpdate = false;
}

bool ULLAnimInstance::ShouldDistanceMatchStop() const
{
	return bHasVelocity && !bHasAcceleration;
}

double ULLAnimInstance::GetPredictedStopDistance() const
{
	return UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
		LastUpdateVelocity,
		bUseSeparateBrakingFriction,
		BrakingFriction,
		GroundFriction,
		BrakingFrictionFactor,
		BrakingDecelerationWalking).Size2D();
}

void ULLAnimInstance::UpdateIdleTurnYawState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationState;
	EAnimNodeReferenceConversionResult ConversionResult;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		if (UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
		{
			TurnYawCurveValue = 0;
		}
		else
		{
			RootYawOffsetMode = ERootYawOffsetMode::Accumulate;
			ProcessTurnYawCurve();
		}
	}
}

void ULLAnimInstance::LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	LandRecoveryAlpha = FMath::GetMappedRangeValueClamped(FVector2f(0, 0.4), FVector2f(0.1, 1.0), TimeFalling); 
}

void ULLAnimInstance::SetupIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	IdleBreakDelayTime = FMath::TruncToInt(FMath::Abs(WorldLocation.X + WorldLocation.Y)) % 10 + 6;
	TimeUntilNextIdleBreak = IdleBreakDelayTime;
}

void ULLAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationState;
	EAnimNodeReferenceConversionResult ConversionResult;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
		{
			if (CanPlayIdleBreak())
			{
				TimeUntilNextIdleBreak -= UAnimExecutionContextLibrary::GetDeltaTime(Context);
			}
			else
			{
				TimeUntilNextIdleBreak = IdleBreakDelayTime;
			}
		}
	}
}

void ULLAnimInstance::SetUpTurnInPlaceRotationState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceRotationDirection = FMath::Sign(RootYawOffset) * -1.f;
}

void ULLAnimInstance::SetUpTurnInPlaceRecoveryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceRecoveryDirection = TurnInPlaceRotationDirection;
}

void ULLAnimInstance::SetUpStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	StartDirection = LocalVelocityDirection;
}

void ULLAnimInstance::UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationState;
	EAnimNodeReferenceConversionResult ConversionResult;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
		{
			RootYawOffsetMode = ERootYawOffsetMode::Hold;
		}
	}
}

void ULLAnimInstance::UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationState;
	EAnimNodeReferenceConversionResult ConversionResult;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
		{
			RootYawOffsetMode = ERootYawOffsetMode::Accumulate;
		}
	}
}

void ULLAnimInstance::SetUpPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotInitialDirection = LocalVelocityDirection;
}

void ULLAnimInstance::UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (LastPivotTime > 0)
	{
		LastPivotTime -= UAnimExecutionContextLibrary::GetDeltaTime(Context);
	}
}

void ULLAnimInstance::UpdateIdleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, IdleAnimSequence);
	}
}

void ULLAnimInstance::SetUpIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequencePlayerLibrary::SetSequence(SequencePlayer, IdleBreakAnimSequences[CurrentIdleBreakIndex]);
		CurrentIdleBreakIndex = (CurrentIdleBreakIndex + 1) % IdleBreakAnimSequences.Num();  
	}
}

void ULLAnimInstance::SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetSequence(
			SequenceEvaluator, SelectDirectionalAnimation(JogStartCardinals, LocalVelocityDirection));
		USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0);
		StrideWarpingStartAlpha = 0;
		
	}
}

void ULLAnimInstance::UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		const float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluator);
		StrideWarpingStartAlpha = FMath::GetMappedRangeValueClamped(
			FVector2D(0, StrideWarpingBlendInDurationScaled), FVector2D(0, 1), ExplicitTime - StrideWarpingBlendInStartOffset);

		const FVector2D PlayRateClamp(
			UKismetMathLibrary::Lerp(StrideWarpingBlendInDurationScaled, PlayRateClampStartsPivots.X, StrideWarpingStartAlpha),
			PlayRateClampStartsPivots.Y);
		UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, SequenceEvaluator, DisplacementSinceLastUpdate, LocomotionDistanceCurveName, PlayRateClamp);
	}
}

void ULLAnimInstance::UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequencePlayerLibrary::SetSequenceWithInertialBlending(
			Context, SequencePlayer, SelectDirectionalAnimation(JogCardinals, LocalVelocityDirectionNoOffset));
		
		UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, DisplacementSpeed, PlayRateClampCycle);
		
		StrideWarpingCycleAlpha = FMath::FInterpTo(
			StrideWarpingCycleAlpha, bIsRunningIntoWall ? 0.5f : 1.0f, UAnimExecutionContextLibrary::GetDeltaTime(Context), 10);
	}
}

void ULLAnimInstance::SetUpStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, SelectDirectionalAnimation(JogStopCardinals, LocalVelocityDirection));
	}
	
	if (!ShouldDistanceMatchStop())
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, 0, LocomotionDistanceCurveName);
	}
}

void ULLAnimInstance::UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (ShouldDistanceMatchStop())
	{
		const double DistanceToMatch = GetPredictedStopDistance();
		if (DistanceToMatch > 0)
		{
			EAnimNodeReferenceConversionResult ConversionResult;
			const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
			if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
			{
				UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, DistanceToMatch, LocomotionDistanceCurveName);
			}
			return;
		}
	}

	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluator);
	}
}

void ULLAnimInstance::SetUpPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotStartingAcceleration = LocalAcceleration2D;

	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, SelectDirectionalAnimation(JogPivotCardinals, CardinalDirectionFromAcceleration));
		USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0);
		StrideWarpingPivotAlpha = 0;
		TimeAtPivotStop = 0;
		LastPivotTime = 0.2;
	}
}

void ULLAnimInstance::UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		const float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluator);

		if (LastPivotTime > 0)
		{
			const TObjectPtr<UAnimSequence> NewDesiredSequence = SelectDirectionalAnimation(JogPivotCardinals, CardinalDirectionFromAcceleration);
			if (NewDesiredSequence != USequenceEvaluatorLibrary::GetSequence(SequenceEvaluator))
			{
				USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluator, NewDesiredSequence);
				PivotStartingAcceleration = LocalAcceleration2D;
			}
		}

		if (FVector::DotProduct(LocalVelocity2D, LocalAcceleration2D) < 0)
		{
			const float DistanceToTarget = UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(
				CurrAcceleration, LastUpdateVelocity, GroundFriction).Size2D();
			UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, DistanceToTarget, LocomotionDistanceCurveName);
			TimeAtPivotStop = ExplicitTime;
		}
		else
		{
			StrideWarpingPivotAlpha = FMath::GetMappedRangeValueClamped(
				FVector2f(0, StrideWarpingBlendInDurationScaled), FVector2f(0, 1),
				ExplicitTime - TimeAtPivotStop - StrideWarpingBlendInStartOffset);
			const FVector2D PlayRateClamp(FMath::Lerp(0.2, PlayRateClampStartsPivots.X, StrideWarpingPivotAlpha), PlayRateClampStartsPivots.Y);

			UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(
				Context, SequenceEvaluator, DisplacementSinceLastUpdate, LocomotionDistanceCurveName, PlayRateClamp);
		}
	}
}

void ULLAnimInstance::SetUpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0);
	}
}

void ULLAnimInstance::UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, GroundDistance, JumpDistanceCurveName);
	}
}

void ULLAnimInstance::SetupTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceAnimTime = 0;
	
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0);
	}
}

void ULLAnimInstance::UpdateTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(
			Context, SequenceEvaluator, SelectTurnInPlaceAnimation(TurnInPlaceRotationDirection));
		
		TurnInPlaceAnimTime += UAnimExecutionContextLibrary::GetDeltaTime(Context);
		USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, TurnInPlaceAnimTime);
	}
}

void ULLAnimInstance::UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, ConversionResult);
	if (ConversionResult == EAnimNodeReferenceConversionResult::Succeeded)
	{
		USequencePlayerLibrary::SetSequenceWithInertialBlending(
			Context, SequencePlayer, SelectTurnInPlaceAnimation(TurnInPlaceRecoveryDirection));
	}
}

void ULLAnimInstance::ProcessTurnYawCurve()
{
	const float PreviousTurnYawCurveValue = TurnYawCurveValue;
	const float TurnYawWeight = GetCurveValue(TEXT("TurnYawWeight"));
	if (FMath::IsNearlyZero(TurnYawWeight))
	{
		TurnYawCurveValue = 0;
	}
	else
	{
		TurnYawCurveValue = GetCurveValue(TEXT("RemainingTurnYaw")) / TurnYawWeight;
		if (PreviousTurnYawCurveValue != 0)
		{
			SetRootYawOffset(RootYawOffset - (TurnYawCurveValue - PreviousTurnYawCurveValue));
		}
	}
}

void ULLAnimInstance::SetRootYawOffset(float InRootYawOffset)
{
	const float NormalizedRootYawOffset = FRotator::NormalizeAxis(InRootYawOffset);
	RootYawOffset = RootYawOffsetAngleClamp.X == RootYawOffsetAngleClamp.Y ?
		NormalizedRootYawOffset :
		FMath::ClampAngle(NormalizedRootYawOffset, RootYawOffsetAngleClamp.X, RootYawOffsetAngleClamp.Y);
}

TObjectPtr<UAnimSequence> ULLAnimInstance::SelectTurnInPlaceAnimation(float Direction) const
{
	return Direction > 0 ? TurnInPlaceRightAnimSequence : TurnInPlaceLeftAnimSequence;
}

float ULLAnimInstance::GetGroundDistance(TObjectPtr<ACharacter> Owner)
{
	static constexpr float GroundTraceDistance = 100000.0f;

	const TObjectPtr<UCharacterMovementComponent> MoveComponent = Owner->GetCharacterMovement();
	
	if (!MoveComponent || (GFrameCounter == LastUpdateFrame))
	{
		return LastGroundDistance;
	}

	if (MoveComponent->MovementMode == MOVE_Walking)
	{
		LastGroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = Owner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (MoveComponent->UpdatedComponent ? MoveComponent->UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(Owner->GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LyraCharacterMovementComponent_GetGroundInfo), false, Owner);
		FCollisionResponseParams ResponseParam;
		MoveComponent->InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		LastGroundDistance = GroundTraceDistance;

		if (MoveComponent->MovementMode == MOVE_NavWalking)
		{
			LastGroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			LastGroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	LastUpdateFrame = GFrameCounter;

	return LastGroundDistance;
}

void ULLAnimInstance::UpdateLocationData(float DeltaTime)
{
	DisplacementSinceLastUpdate = (PrevWorldLocation - WorldLocation).Size2D();
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaTime);

	if (bIsFirstUpdate)
	{
		DisplacementSinceLastUpdate = 0;
		DisplacementSpeed = 0;
	}
}

bool ULLAnimInstance::CanPlayIdleBreak() const
{
	return !IdleBreakAnimSequences.IsEmpty() && !(bIsAnyMontagePlaying || bHasVelocity);
}

bool ULLAnimInstance::IsMovingPerpendicularToInitialPivot() const
{
	return
		((PivotInitialDirection == ECardinalDirection::Forward || PivotInitialDirection == ECardinalDirection::Backward) &&
		!(LocalVelocityDirection == ECardinalDirection::Forward || LocalVelocityDirection == ECardinalDirection::Backward)) ||
		((PivotInitialDirection == ECardinalDirection::Left || PivotInitialDirection == ECardinalDirection::Right) &&	
		!(LocalVelocityDirection == ECardinalDirection::Left || LocalVelocityDirection == ECardinalDirection::Right));
}

void ULLAnimInstance::UpdateAccelerationData()
{
	const FVector WorldAcceleration2D(CurrAcceleration.X, CurrAcceleration.Y, 0);
	LocalAcceleration2D = WorldRotation.UnrotateVector(WorldAcceleration2D);
	bHasAcceleration = !FMath::IsNearlyZero(LocalAcceleration2D.SizeSquared2D());

	PivotDirection2D = FMath::Lerp(PivotDirection2D, WorldAcceleration2D.GetSafeNormal(), 0.5f).GetSafeNormal();

	const float Angle = UKismetAnimationLibrary::CalculateDirection(PivotDirection2D, WorldRotation);
	const ECardinalDirection CurrentDirection = SelectCardinalDirectionFromAngle(Angle, CardinalDirectionDeadZone, ECardinalDirection::Forward, false);
	CardinalDirectionFromAcceleration = GetOppositeCardinalDirection(CurrentDirection);
}

void ULLAnimInstance::UpdateRotationData()
{
	YawDeltaSinceLastUpdate = WorldRotation.Yaw - PrevWorldRotation.Yaw;
	const float YawDeltaSpeed = UKismetMathLibrary::SafeDivide(YawDeltaSinceLastUpdate, GetDeltaSeconds());
	AdditiveLeanAngle = YawDeltaSpeed * 0.0375;

	if (bIsFirstUpdate)
	{
		YawDeltaSinceLastUpdate = 0;
		AdditiveLeanAngle = 0;
	}
}

void ULLAnimInstance::UpdateVelocityData()
{
	bWasMovingLastUpdate = !LocalVelocity2D.IsZero();
	
	const FVector WorldVelocity2D(WorldVelocity.X, WorldVelocity.Y, 0);
	LocalVelocity2D = WorldRotation.UnrotateVector(WorldVelocity2D);

	LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(WorldVelocity2D, WorldRotation);
	LocalVelocityDirectionAngleWithOffset = LocalVelocityDirectionAngle - RootYawOffset;

	LocalVelocityDirection = SelectCardinalDirectionFromAngle(
		LocalVelocityDirectionAngleWithOffset, CardinalDirectionDeadZone, LocalVelocityDirection, bWasMovingLastUpdate);
	LocalVelocityDirectionNoOffset = SelectCardinalDirectionFromAngle(
		LocalVelocityDirectionAngle, CardinalDirectionDeadZone, LocalVelocityDirectionNoOffset, bWasMovingLastUpdate);
	
	bHasVelocity = !FMath::IsNearlyZero(LocalVelocity2D.SizeSquared2D());
}

void ULLAnimInstance::UpdateRootYawOffset(float InDeltaTime)
{
	switch (RootYawOffsetMode)
	{
	case ERootYawOffsetMode::Accumulate:
		SetRootYawOffset(RootYawOffset - YawDeltaSinceLastUpdate);
		break;
	case ERootYawOffsetMode::BlendOut:
		SetRootYawOffset(UKismetMathLibrary::FloatSpringInterp(
			RootYawOffset, 0, RootYawOffsetSpringState, 80, 1, InDeltaTime, 1, 0.5));
		break;
	default:
		break;
	}
	RootYawOffsetMode = ERootYawOffsetMode::BlendOut;
}

TObjectPtr<UAnimSequence> ULLAnimInstance::SelectDirectionalAnimation(const FCardinalDirections& Cardinals,
	ECardinalDirection Direction)
{
	switch (Direction)
	{
	default:
		// falls through
	case ECardinalDirection::Forward:
		return Cardinals.Forward;
	case ECardinalDirection::Backward:
		return Cardinals.Backward;
	case ECardinalDirection::Left:
		return Cardinals.Left;
	case ECardinalDirection::Right:
		return Cardinals.Right;
	}
}

ECardinalDirection ULLAnimInstance::SelectCardinalDirectionFromAngle(float Angle, float DeadZone,
	ECardinalDirection CurrentDirection, bool bUseCurrentDirection)
{
	const float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;

	if (bUseCurrentDirection)
	{
		switch (CurrentDirection)
		{
		case ECardinalDirection::Forward:
			FwdDeadZone *= 2;
			break;
		case ECardinalDirection::Backward:
			BwdDeadZone *= 2;
			break;
		default:
			break;
		}
	}

	if (AbsAngle <= FwdDeadZone + 45)
	{
		return ECardinalDirection::Forward;
	}
	else if (AbsAngle >= 135 - BwdDeadZone)
	{
		return ECardinalDirection::Backward;
	}
	else if (Angle > 0)
	{
		return ECardinalDirection::Right;
	}
	else
	{
		return ECardinalDirection::Left;
	}
}

ECardinalDirection ULLAnimInstance::GetOppositeCardinalDirection(ECardinalDirection CurrentDirection)
{
	switch (CurrentDirection)
	{
	default:
		// falls through
	case ECardinalDirection::Forward:
		return ECardinalDirection::Backward;
	case ECardinalDirection::Backward:
		return ECardinalDirection::Forward;
	case ECardinalDirection::Left:
		return ECardinalDirection::Right;
	case ECardinalDirection::Right:
		return ECardinalDirection::Left;
	}
}
