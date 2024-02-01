
#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "Components/ActorComponent.h"
#include "ShooterProject/HUD/BlasterHUD.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

enum class EWeaponType : uint8;
class AWeapon;
class AALSCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHOOTERPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
		UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	friend class AALSCharacter;
	friend class AALSBaseCharacter;
protected:
	virtual void BeginPlay() override;

	void FireButtonPressed(bool bPressed);
	void Fire();
	
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrossHairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float DeltaTime);

private:
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UPROPERTY()
	AALSCharacter* BlasterCharacter;

	UPROPERTY(Replicated)
	bool bAiming;
	
	bool bFireButtonPressed;


	FVector HitTarget;
	FHUDPackage HUDPackage;

	/** 
	* HUD and crosshairs
	*/

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;

	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	UPROPERTY()
	class AALSPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	/**
	* Automatic fire
	*/

	FTimerHandle FireTimer;
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	void InitializeCarriedAmmo();
	
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

public:
	void EquipWeapon(AWeapon* WeaponToEquip);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FHitResult HitResult;

	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

protected:
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	
};
