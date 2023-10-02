// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

UCLASS()
class PORTAL_API APortalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalActor();

	void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Default Scene Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class USceneComponent> DefaultSceneRoot;

	// Spline
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class USplineComponent> Spline;

	// Portal entrance door
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr <class UStaticMeshComponent> Entrance;

	// Portal exit door
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UStaticMeshComponent> Exit;
	
	// Protal transmit trigger
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UBoxComponent> TransmitTrigger;

	// The scene capture component of portal exit 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class USceneCaptureComponent2D> ExitSceneCapture;

	// Effect component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UNiagaraComponent> EntranceEffect;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UNiagaraComponent> ExitEffect;

	// The information of character`s transition
	TMap<class ACharacter*, float> TransmittingCharacterInfos;

	/// <summary>
	///  Create protal tunnel by spline component
	/// </summary>
	void CreatePortalTunnel();

	/// <summary>
	/// Set up the door of portal by spline component
	/// </summary>
	void SetupPortalDoor();

	/// <summary>
	/// The event when character overlap on protal transmit trigger
	/// </summary>
	UFUNCTION()
		void OnPortalTransmitTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/// <summary>
	/// Transmit character when character in portal
	/// </summary>
	UFUNCTION(BlueprintCallable)
		void Transmit();

public:
	// Portal collition type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		TEnumAsByte<ECollisionEnabled::Type> CollisionType;

	// The mesh of portal door
	UPROPERTY(EditAnywhere, Category = Portal)
		TObjectPtr<UStaticMesh> DoorMesh;

	// Rotation of portal door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		FRotator DoorRotation;

	// The class of character which can use the portal
	UPROPERTY(EditAnywhere, Category = Portal)
		TSubclassOf<ACharacter> PlayerClass;

	// The speed of portal transition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		float PortalSpeed;

	// The launch power when chacter get out of portal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		float PortalOutPower;

};
