// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Recoil.generated.h"

// This class does not need to be modified.
UINTERFACE()
class URecoil : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SHOOTERPROJECT_API IRecoil
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void IF_Recoil(float Strength, float Delay, float TimeBeforeStart);
};
