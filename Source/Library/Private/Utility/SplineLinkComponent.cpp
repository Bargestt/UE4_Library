// Fill out your copyright notice in the Description page of Project Settings.

#include "SplineLinkComponent.h"
#include "Kismet/KismetSystemLibrary.h"






// Sets default values for this component's properties
USplineLinkComponent::USplineLinkComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;


	SetMobility(EComponentMobility::Movable);
	TransformUpdated.AddUObject(this, &USplineLinkComponent::OnTransformUpdated);
	//0.55191502449
	TangentScale = 0.55191502449 * 3;
	// ...
}

void USplineLinkComponent::OnRegister()
{
	Super::OnRegister();

	if (TargetComponent)
	{
		UpdateLink();
	}
}


#if WITH_EDITOR
void USplineLinkComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("PropChange"));

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USplineLinkComponent, TargetActor))
	{
		if (IsValid(TargetActor))
		{
			SetTarget(TargetActor->GetRootComponent());
		}
	}
}
#endif




void USplineLinkComponent::OnTransformUpdated(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	if (TargetComponent)
	{
		UpdateLink();
	}
}

void USplineLinkComponent::UpdateLink()
{
	const FTransform& Tr = GetComponentTransform();
	FVector EndPosition = Tr.InverseTransformPosition(TargetComponent->GetComponentLocation());

	FVector StartTangent;
	switch (StartPointStyle)
	{
	default:
		//Fall through for default case
	case ESplineLinkAxisMode::SL_Auto:
	{
		float AbsMin = EndPosition.GetAbsMin();
		bool X_IsMin = (FMath::Abs(EndPosition.X) == AbsMin) ? 1 : 0;
		bool Y_IsMin = (FMath::Abs(EndPosition.Y) == AbsMin) ? 1 : 0;
		bool Z_IsMin = (FMath::Abs(EndPosition.Z) == AbsMin) ? 1 : 0;
		StartTangent = FVector(X_IsMin, Y_IsMin, Z_IsMin) * EndPosition;
	}
		break;
	case ESplineLinkAxisMode::SL_Rotation:
		StartTangent = FVector::ForwardVector * EndPosition.GetSafeNormal();
		break;
	case ESplineLinkAxisMode::SL_X:
		StartTangent = FVector(EndPosition.X, 0, 0);
		break;
	case ESplineLinkAxisMode::SL_Y:
		StartTangent = FVector(0, EndPosition.Y, 0);
		break;
	case ESplineLinkAxisMode::SL_Z:
		StartTangent = FVector(0, 0, EndPosition.Z);
		break;
	}

	FVector EndTangent;
	switch (EndPointStyle)
	{
	default:
		//Fall through for default case
	case ESplineLinkAxisMode::SL_Auto:
	{
		float AbsMin = EndPosition.GetAbsMin();
		bool X_IsMin = (FMath::Abs(EndPosition.X) == AbsMin) ? 1 : 0;
		bool Y_IsMin = (FMath::Abs(EndPosition.Y) == AbsMin) ? 1 : 0;
		bool Z_IsMin = (FMath::Abs(EndPosition.Z) == AbsMin) ? 1 : 0;
		EndTangent = FVector(X_IsMin, Y_IsMin, Z_IsMin) * EndPosition;
	}
		break;
	case ESplineLinkAxisMode::SL_Rotation:
		EndTangent = TargetComponent ? Tr.InverseTransformVector(TargetComponent->GetForwardVector()) * EndPosition.GetSafeNormal() : FVector::ZeroVector;
		break;
	case ESplineLinkAxisMode::SL_X:
		EndTangent = FVector(EndPosition.X, 0, 0);
		break;
	case ESplineLinkAxisMode::SL_Y:
		EndTangent = FVector(0, EndPosition.Y, 0);
		break;
	case ESplineLinkAxisMode::SL_Z:
		EndTangent = FVector(0, 0, EndPosition.Z);
		break;
	}

	StartTangent *= TangentScale;
	EndTangent *= TangentScale;

	SetParams(StartTangent, EndPosition, EndTangent);
}



void USplineLinkComponent::SetTarget(USceneComponent* NewTarget)
{
	if (NewTarget)
	{
		if (NewTarget != TargetComponent)
		{
			RemoveTarget();
			TargetComponent = NewTarget;
			Delegate = TargetComponent->TransformUpdated.AddUObject(this, &USplineLinkComponent::OnTransformUpdated);
		}
	}
	else
	{
		RemoveTarget();
	}
}

void USplineLinkComponent::RemoveTarget()
{
	if (TargetComponent)
	{
		TargetComponent->TransformUpdated.Remove(Delegate);
	}
	TargetComponent = nullptr;
}
#if !UE_BUILD_SHIPPING
FPrimitiveSceneProxy* USplineLinkComponent::CreateSceneProxy()
{
	return nullptr;
}
#endif

void USplineLinkComponent::SetParams(const FVector& StartTangent, const FVector& EndPosition, const FVector& EndTangent)
{

}


