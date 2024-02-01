// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AI/ALSAIController.h"
#include "Character/ALSPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "ShooterProject/CombatComponent/CombatComponent.h"
#include "ShooterProject/GameMode/BlasterGameMode.h"
#include "ShooterProject/PlayerState/BlasterPlayerState.h"
#include "ShooterProject/Weapons/Weapon.h"

AALSCharacter::AALSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HeldObjectRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HeldObjectRoot"));
	HeldObjectRoot->SetupAttachment(GetMesh());

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(HeldObjectRoot);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(HeldObjectRoot);

	AIControllerClass = AALSAIController::StaticClass();
	

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void AALSCharacter::ClearHeldObject()
{
	StaticMesh->SetStaticMesh(nullptr);
	SkeletalMesh->SetSkeletalMesh(nullptr);
	SkeletalMesh->SetAnimInstanceClass(nullptr);
}

void AALSCharacter::AttachToHand(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh, UClass* NewAnimClass,
                                 bool bLeftHand, FVector Offset)
{
	ClearHeldObject();

	if (IsValid(NewStaticMesh))
	{
		StaticMesh->SetStaticMesh(NewStaticMesh);
	}
	else if (IsValid(NewSkeletalMesh))
	{
		SkeletalMesh->SetSkeletalMesh(NewSkeletalMesh);
		if (IsValid(NewAnimClass))
		{
			SkeletalMesh->SetAnimInstanceClass(NewAnimClass);
		}
	}

	FName AttachBone;
	if (bLeftHand)
	{
		AttachBone = TEXT("VB LHS_ik_hand_gun");
	}
	else
	{
		AttachBone = TEXT("VB RHS_ik_hand_gun");
	}

	HeldObjectRoot->AttachToComponent(GetMesh(),
	                                  FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachBone);
	HeldObjectRoot->SetRelativeLocation(Offset);
}

void AALSCharacter::RagdollStart()
{
	ClearHeldObject();
	Super::RagdollStart();
}

void AALSCharacter::RagdollEnd()
{
	Super::RagdollEnd();
	UpdateHeldObject();
}

ECollisionChannel AALSCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	const FName CameraSocketName = bRightShoulder ? TEXT("TP_CameraTrace_R") : TEXT("TP_CameraTrace_L");
	TraceOrigin = GetMesh()->GetSocketLocation(CameraSocketName);
	TraceRadius = 15.0f;
	return ECC_Camera;
}

FTransform AALSCharacter::GetThirdPersonPivotTarget()
{
	return FTransform(GetActorRotation(),
	                  (GetMesh()->GetSocketLocation(TEXT("Head")) + GetMesh()->GetSocketLocation(TEXT("root"))) / 2.0f,
	                  FVector::OneVector);
}

FVector AALSCharacter::GetFirstPersonCameraTarget()
{
	return GetMesh()->GetSocketLocation(TEXT("FP_Camera"));
}

void AALSCharacter::OnOverlayStateChanged(EALSOverlayState PreviousState)
{
	Super::OnOverlayStateChanged(PreviousState);
	UpdateHeldObject();
}

void AALSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHeldObjectAnimations();

	PollInit();
	
}

void AALSCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHeldObject();

	/*
	 * SET HEALTH
	 */
	
	UpdateHUDHealth();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AALSCharacter::ReceiveDamage);
	}
}


////////////////////////////////////////////////////////////////////////////

void AALSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->BlasterCharacter = this;
	}
}

void AALSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AALSCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AALSCharacter,HeldObjectRoot);
	DOREPLIFETIME(AALSCharacter, Health);
}

void AALSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&AALSCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire",IE_Released,this,&AALSCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Equip",IE_Pressed,this,&AALSCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AALSCharacter::ReloadButtonPressed);
}

void AALSCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
	}
	
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AALSCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}


void AALSCharacter::EquipButtonPressed()
{
	if (CombatComponent) 
	{
		//ServerEquippedButtonPressed();
		
		if(HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
			// when E Key is Pressed EquipWeapon() only get called if we have authority
		}
		else
		{
			ServerEquippedButtonPressed();
			// if we do not have Authority we call/send ServerEquippedButtonPressed() RPC so that server can call EquipWeapon function
		}
	}
}

void AALSCharacter::ServerEquippedButtonPressed_Implementation()
{
	if (CombatComponent) //No Need for HasAuthority() because it will be executed in server
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AALSCharacter::FireButtonPressed() 
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void AALSCharacter::FireButtonReleased() 
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void AALSCharacter::ReloadButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void AALSCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			if(CombatComponent->bAiming == true)
				AnimInstance->Montage_Play(IronSightReloadMontage);
			else
				AnimInstance->Montage_Play(HipReloadMontage);
			break;
			
		default:
			break;
		}
	}
}

void AALSCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
								  AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();

	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<AALSPlayerController>(Controller) : BlasterPlayerController;
			AALSPlayerController* AttackerController = Cast<AALSPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}


void AALSCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<AALSPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AALSCharacter::OnRep_Health()
{
	UpdateHUDHealth();
}

void AALSCharacter::Elim()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}
	
	MulticastElim();

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AALSCharacter::ElimTimerFinished,
		ElimDelay
	);

}

void AALSCharacter::MulticastElim_Implementation()
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bElimmed = true;

	DeathRagDoll_Implementation(CombatComponent->HitResult.ImpactPoint,CombatComponent->HitResult.ImpactNormal,100.f,
		CombatComponent->HitResult.BoneName);
	
	// do ragdoll
	GEngine->AddOnScreenDebugMessage(-1,10.f,FColor::Red,TEXT("PLayer Elimanated"));

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}
	// Disable collision
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AALSCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void AALSCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}

ECombatState AALSCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX;
	return CombatComponent->CombatState;
}

FVector AALSCharacter::GetFirstPersonCameraTargetADS()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		return CombatComponent->EquippedWeapon->GetWeaponMesh()->GetSocketLocation(TEXT("ADS"));
	}

	return {};
}

void AALSCharacter::DeathRagDoll_Implementation(FVector Location, FVector Impulse, float ImpulseMultiplier,
	FName HitBone)
{
	Multi_DeathHitRagDoll_Implementation(Location,Impulse,ImpulseMultiplier,HitBone);
}

void AALSCharacter::Multi_DeathHitRagDoll_Implementation(FVector Location, FVector Impulse, float ImpulseMultiplier,
	FName HitBone)
{
	if (MovementState == EALSMovementState::None || MovementState == EALSMovementState::Grounded || MovementState ==
		EALSMovementState::InAir || MovementState == EALSMovementState::Mantling)
	{
		RagdollStart();

		if (HasAuthority())
		{
			Multicast_RagdollStart();

			GetMesh()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
			GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
			//GetMesh()->SetSimulatePhysics(true);
			GetMesh()->AddImpulseAtLocation(Impulse * (ImpulseMultiplier*-100.f),Location,HitBone);
		}
		else
		{
			Server_RagdollStart();

			GetMesh()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
			GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
			//GetMesh()->SetSimulatePhysics(true);
			GetMesh()->AddImpulseAtLocation(Impulse * (ImpulseMultiplier*-100.f),Location,HitBone);
		}
		
	}
}
