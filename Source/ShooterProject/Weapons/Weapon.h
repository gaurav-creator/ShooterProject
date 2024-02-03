// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"
class UCameraComponent;
class AALSCharacter;

UENUM(BlueprintType)
enum class EWeaponStates : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),
	
	EWS_Max UMETA(DisplayName = "DefaultMAX"),
};
UCLASS()
class SHOOTERPROJECT_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	/**
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	/** 
	* Automatic fire
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;
	
protected:
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	UCameraComponent* CameraComponent;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	
private:

	/*UPROPERTY(VisibleAnywhere,Category = "Weapon Properties")
	USceneComponent* DefaultSceneComponent;*/
	
	UPROPERTY(VisibleAnywhere,Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere,Category = "Weapon Properties")
	class USphereComponent* AreaSphere;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState,VisibleAnywhere)
	EWeaponStates WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY()
	class AALSCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class AALSPlayerController* BlasterOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	AWeapon();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void ShowPickupWidget(bool bShowWidget);
	
	virtual void Fire(const FVector& HitTarget);
	
	void SetWeaponState(EWeaponStates State);

	virtual void OnRep_Owner() override;
	
	void SetHUDAmmo();

	bool IsEmpty();
	
	void Dropped();

	void AddAmmo(int32 AmmoToAdd);

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	
	FORCEINLINE USphereComponent* GetAreaSphere() const {return  AreaSphere;}
	
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	/*
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	double Recoil;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	double SideRecoil;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	double KickBack;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	double KickSide;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	double KickUp;*/

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	float Strength;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	float Delay;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	float TimeBeforeStart;
	

};
