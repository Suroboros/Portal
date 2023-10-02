// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalActor.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "../PortalCharacter.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SceneCaptureComponent2D.h"
#include "NiagaraComponent.h"

// Sets default values
APortalActor::APortalActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create default scene and set it to root
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// Create Spline
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("PortalSpline"));
	Spline->SetupAttachment(DefaultSceneRoot);

	// Create portal door
	Entrance = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalEntranceDoor"));
	Exit = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalExitDoor"));

	// Create portal trigger
	TransmitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalTransmitTrigger"));
	TransmitTrigger->SetupAttachment(Entrance);
	TransmitTrigger->OnComponentBeginOverlap.AddDynamic(this, &APortalActor::OnPortalTransmitTriggerBeginOverlap);

	// Create scene capture component which capture the scene of exit
	ExitSceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PortalExitSceneCapture"));
	ExitSceneCapture->SetupAttachment(Exit);

	// Create effect component and attach them
	EntranceEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalEntranceEffect"));
	EntranceEffect->SetupAttachment(Entrance);
	ExitEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalExitEffect"));
	ExitEffect->SetupAttachment(Exit);

	// Parameter Initialize
	DoorRotation = FRotator(0, 90, 0);
	PortalSpeed = 100.f;
	PortalOutPower = 100.f;
}

void APortalActor::OnConstruction(const FTransform& Transform)
{
	// Create portal tunnel by spline
	CreatePortalTunnel();

	// Set portal door posiontion and rotation
	SetupPortalDoor();
}

// Called when the game starts or when spawned
void APortalActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APortalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APortalActor::CreatePortalTunnel()
{
	check(Spline);

	UStaticMesh* SM_Tunnel = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Game/Portal/Meshes/SM_Tunnel")));

	for (int i = 0; i <= Spline->GetNumberOfSplinePoints() - 2; i++)
	{
		// Get spline step start and end
		FVector StartPos, StartTangent, EndPos, EndTangent;
		Spline->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
		Spline->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);

		// Create spline mesh with spline step start and end
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this,USplineMeshComponent::StaticClass());
		SplineMesh->SetStaticMesh(SM_Tunnel);
		SplineMesh->SetMobility(EComponentMobility::Movable);
		SplineMesh->RegisterComponentWithWorld(GetWorld());
		SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SplineMesh->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		SplineMesh->SetCollisionEnabled(CollisionType);
	}
}

void APortalActor::SetupPortalDoor()
{
	check(Spline);
	check(Entrance);
	check(Exit);
	check(DoorMesh);

	UMaterial* portalEntranceMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Game/Portal/Materials/M_PortalEntranceDoor"));
	UMaterial* portalExitMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Game/Portal/Materials/M_PortalExitDoor"));
	check(portalEntranceMaterial);
	check(portalExitMaterial);

	// Get start and end of spline
	FTransform StartPointTransform = Spline->GetTransformAtSplinePoint(0,ESplineCoordinateSpace::Local);
	FTransform EndPointTransform = Spline->GetTransformAtSplinePoint(Spline->GetNumberOfSplinePoints()-1, ESplineCoordinateSpace::Local);
	
	// Set portal entrance
	Entrance->SetStaticMesh(DoorMesh);
	Entrance->SetMobility(EComponentMobility::Movable);
	Entrance->SetRelativeTransform(StartPointTransform);
	Entrance->AddRelativeRotation(DoorRotation);
	Entrance->SetCollisionEnabled(CollisionType);
	Entrance->SetMaterial(0, portalEntranceMaterial);
	Entrance->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
	
	// Set portal exit
	Exit->SetStaticMesh(DoorMesh);
	Exit->SetMobility(EComponentMobility::Movable);
	Exit->SetRelativeTransform(EndPointTransform);
	Exit->AddRelativeRotation(DoorRotation + FRotator(0,180,0));
	Exit->SetCollisionEnabled(CollisionType);
	Exit->SetMaterial(0, portalExitMaterial);
	Exit->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);

	// Set up trigger and scene capture
	TransmitTrigger->SetWorldTransform(Spline->GetTransformAtSplinePoint(0, ESplineCoordinateSpace::World));
	ExitSceneCapture->SetWorldTransform(Spline->GetTransformAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World));

	// Set up effects
	EntranceEffect->SetWorldTransform(Spline->GetTransformAtSplinePoint(0, ESplineCoordinateSpace::World));
	EntranceEffect->AddRelativeLocation(EntranceEffect->GetForwardVector());
	ExitEffect->SetWorldTransform(Spline->GetTransformAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World));
}

void APortalActor::OnPortalTransmitTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check the overlaped actor can use this portal
	if (OtherActor != nullptr && OtherActor->GetClass()->IsChildOf(PlayerClass))
	{
		// Set up the information of portal transition before transition begin
		APortalCharacter* player = Cast<APortalCharacter>(OtherActor);
		TransmittingCharacterInfos.Add(player,0);

		// Start portal transtion
		Transmit();
	}
}

void APortalActor::Transmit()
{
	if (TransmittingCharacterInfos.IsEmpty())
	{
		return;
	}
	
	TArray<TPair<ACharacter*, float>> TransmittingCharacterInfoArray = TransmittingCharacterInfos.Array();
	// All the character in the portal
	for (int i = 0;i< TransmittingCharacterInfos.Num();++i)
	{
		TPair<ACharacter*, float> characterTransmitInfo = TransmittingCharacterInfoArray[i];
		// Set character movement mode
		UCharacterMovementComponent* playerMovement = characterTransmitInfo.Key->GetCharacterMovement();
		playerMovement->SetMovementMode(EMovementMode::MOVE_None);

		//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("%f"), TransmittingCharacterInfos[characterTransmitInfo.Key]));

		float portalLength = Spline->GetSplineLength();

		// Check portal transition is finished by compare the lengh of portal and the distance of transition
		if (TransmittingCharacterInfos[characterTransmitInfo.Key] <= portalLength)
		{
			// Set up latent action
			FLatentActionInfo info;
			info.CallbackTarget = this;
			info.UUID = GetUniqueID();
			info.ExecutionFunction = FName("Transmit");
			info.Linkage = 0;

			// record the distance of portal transition
			TransmittingCharacterInfos[characterTransmitInfo.Key] += PortalSpeed;

			// Move character
			FTransform transform = Spline->GetTransformAtDistanceAlongSpline(characterTransmitInfo.Value, ESplineCoordinateSpace::World);
			//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, transform.GetLocation().ToString());
			UKismetSystemLibrary::MoveComponentTo(characterTransmitInfo.Key->GetRootComponent(), transform.GetLocation(), transform.Rotator(), false, false, 0.2f, false, EMoveComponentAction::Move, info);
		}
		else
		{
			// Set movement mode and delete the information of transition after portal transition finished
			playerMovement->SetMovementMode(EMovementMode::MOVE_Walking);
			characterTransmitInfo.Key->LaunchCharacter(characterTransmitInfo.Key->GetActorForwardVector() * PortalOutPower, false, false);
			TransmittingCharacterInfos.Remove(characterTransmitInfo.Key);
		}
	}
}

