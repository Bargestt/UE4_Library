// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineMeshComponent.h"
#include "SplineLinkComponent.generated.h"


UENUM(BlueprintType)
enum class ESplineLinkAxisMode : uint8
{
	/** Auto direction calculation */
	SL_Auto UMETA(DisplayName = "Auto"),
	/** Use Component rotation */
	SL_Rotation UMETA(DisplayName = "Rotation"),
	/** Align along global X  */
	SL_X UMETA(DisplayName = "X"),
	/** Align along global Y */
	SL_Y UMETA(DisplayName = "Y"),
	/** Align along global Z */
	SL_Z UMETA(DisplayName = "Z")
};

UCLASS( ClassGroup=(Utility), meta=(BlueprintSpawnableComponent), hideCategories=("StaticMesh", "Materials") )
class LIBRARY_API USplineLinkComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	ESplineLinkAxisMode StartPointStyle;

	UPROPERTY(EditAnywhere)
	ESplineLinkAxisMode EndPointStyle;

	UPROPERTY(EditAnywhere)
	float TangentScale;

	UPROPERTY(EditAnywhere)
	AActor* TargetActor;


private:
	FInterpCurveVector Spline;


	UPROPERTY()
	USceneComponent* TargetComponent;

	FDelegateHandle Delegate;

public:
	// Sets default values for this component's properties
	USplineLinkComponent();


	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void OnTransformUpdated(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);

	void UpdateLink();

	void SetTarget(USceneComponent* NewTarget);

	void RemoveTarget();


#if !UE_BUILD_SHIPPING
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
#endif

protected:

	void SetParams(const FVector& StartTangent, const FVector& EndPosition, const FVector& EndTangent);
	
};
