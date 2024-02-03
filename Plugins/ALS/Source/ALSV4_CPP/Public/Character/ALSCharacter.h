
#pragma once

#include "CoreMinimal.h"
#include "Character/ALSBaseCharacter.h"
#include "ShooterProject/CombatComponent/CombatState.h"
#include "ALSCharacter.generated.h"

class AALSPlayerController;
class AALSPlayerCameraManager;
class UCombatComponent;
/**
 * Specialized character class, with additional features like held object etc.
 */
UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API AALSCharacter : public AALSBaseCharacter
{
	GENERATED_BODY()

public:
	AALSCharacter(const FObjectInitializer& ObjectInitializer);

	/** Implemented on BP to update held objects */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|HeldObject")
	void UpdateHeldObject();

	UFUNCTION(BlueprintCallable, Category = "ALS|HeldObject")
	void ClearHeldObject();

	UFUNCTION(BlueprintCallable, Category = "ALS|HeldObject")
	void AttachToHand(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh,
	                  class UClass* NewAnimClass, bool bLeftHand, FVector Offset);

	virtual void RagdollStart() override;

	virtual void RagdollEnd() override;

	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius) override;

	virtual FTransform GetThirdPersonPivotTarget() override;

	virtual FVector GetFirstPersonCameraTarget() override;

protected:
	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	virtual void OnOverlayStateChanged(EALSOverlayState PreviousState) override;

	/** Implement on BP to update animation states of held objects */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|HeldObject")
	void UpdateHeldObjectAnimations();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Component",Replicated)
	TObjectPtr<USceneComponent> HeldObjectRoot = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Component")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh = nullptr;

private:
	bool bNeedsColorReset = false;


	/////////////////////////////////////////////////////////
public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PostInitializeComponents() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetOverlappingWeapon(AWeapon* Weapon);

	/*virtual FVector GetFirstPersonCameraTargetADS() override;*/
	
	void Elim();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE AALSPlayerController* GetBlasterPlayerController() const {return  BlasterPlayerController;}

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HipReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* IronSightReloadMontage;

	void ReloadButtonPressed();

	void PlayReloadMontage();

	ECombatState GetCombatState() const;
	
protected:
	
	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

	void FireButtonPressed() ;
	void FireButtonReleased() ;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	                   class AController* InstigatorController, AActor* DamageCauser);
	
	void UpdateHUDHealth();

	// Poll for any relelvant classes and initialize our HUD
	void PollInit();

	UFUNCTION(Server,Reliable)
	void DeathRagDoll(FVector Location, FVector Impulse, float ImpulseMultiplier,FName HitBone);

	UFUNCTION(NetMulticast,Reliable)
	void Multi_DeathHitRagDoll(FVector Location, FVector Impulse, float ImpulseMultiplier,FName HitBone);
	

private:
	
	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquippedButtonPressed();
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	/**
		* Player health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	AALSPlayerController* BlasterPlayerController;
	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
	
	
};
